#!/usr/bin/perl -w /* -*- indent-tabs-mode: nil -*- */

#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Change MSVC mangled C++ names from 32-bit form to the corresponding
# 64-bit form.  Each line of input can contain at most one mangled
# name.

# Based on experimentation with MSVC2008 and the following web pages:

# http://www.geoffchappell.com/viewer.htm?doc=studies/msvc/language/decoration/index.htm
# Thorough but incomplete. Still, describes details the below sources
# don't mention.

# http://cvs.winehq.com/cvsweb/wine/dlls/msvcrt/undname.c
# Wine's __unDname function, presumably the most complete, although
# not really written to act as "documentation"

# http://mearie.org/documents/mscmangle/
# Relatively complete but a bit badly structured and terse.

# http://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B_Name_Mangling
# seems to be mostly a ripoff on the mearie.org page

# Example transformation:
# ??0ORealDynamicLoader@salhelper@@IAE@PAPAV01@ABVOUString@rtl@@1PAX2@Z =>
# ??0ORealDynamicLoader@salhelper@@IEAA@PEAPEAV01@AEBVOUString@rtl@@1PEAX2@Z

# It should be relatively easy to remove the modification parts of the
# below code and use the regex for some other task on MSVC mangled
# names.

# The regular expression below accepts also nonsensical mangled names,
# so it should not be used to verify correctness of mangled names.

use strict;

my @opstack = ();

sub parse_number($)
{
  my ($num) = @_;

  return $num + 1 if ($num eq '0' || ($num ge '1' && $num le '9'));

  $num =~ tr/ABCDEFGHIJKLMNOP@/0123456789ABCDEF /;
  hex($num);
}

sub format_number($)
{
  my ($num) = @_;

  return $num - 1 if ($num <= 10);

  $num = sprintf("%X", $num);
  $num =~ tr/0123456789ABCDEF/ABCDEFGHIJKLMNOP/;
  $num.'@';
}

sub double_thunk($$)
{
  my ($number, $position) = @_;

  my $bytes = parse_number($number);
  $bytes *= 2;
  push(@opstack, 'r '.($position - length($number)).' '.length($number).' '.format_number($bytes));
}

while (<>)
  {
    m/
      # Named subpattern definitions. I use names of the form
      # __CamelCase__ for the named subpatters so that they are easier
      # to see.
      (?(DEFINE)
        (?<__Number__>
          \?? ([0-9] | [A-P]+@)
        )
        (?<__32BitChecksum__>
          [A-P]{8}@
        )
        (?<__CallingConvention__>
          (?:
            [AB]
            |
            [C-L]
              (?{ push(@opstack, 'r '.(pos()-1).' 1 A cdecl'); })
          )
        )
        (?<__StringLiteralText__>
          (?:
            [_a-zA-Z0-9]
            |
            \?\$[A-P][A-P]
            |
            \?[0-9A-Za-z]
          ){1,20}
        )
        (?<__Identifier__>
          [_a-zA-Z\$][_a-zA-Z0-9\$]*@
        )
        (?<__ArgsZTerminated__>
          (?&__DataTypeInArgs__)+ @? Z
        )
        (?<__ArgsNonZTerminated__>
          (?&__DataTypeInArgs__)+ @?
        )
        (?<__TemplateName__>
          (?&__Identifier__) (?&__ArgsNonZTerminated__)
        )
        (?<__Class__>
          (?:
            [0-9]
            |
            \?\$ (?&__TemplateName__)
            |
            (?&__Identifier__)
          )+@
        )
        (?<__DataTypeCommon__>
          (?:
            # extended types like _int64, bool and wchar_t
            _[D-NW]
            |
            # simple types
            [C-KMNOXZ]
            |
            # class, struct, union, cointerface
            [TUVY] (?&__Class__)
            |
            # references
            [AB]
              (?{ push(@opstack, 'i '.pos().' E reference'); })
              (?&__ModifiedType__)
            |
            # pointers
            [QRS]
              (?{ push(@opstack, 'i '.pos().' E pointer'); })
              (?&__ModifiedType__)
            |
            P
              (?:
                # function pointer
                6 (?&__CallingConvention__) (?&__DataTypeNotInArgs__) (?&__ArgsZTerminated__)
                |
                # other pointer
                (?{ push(@opstack, 'i '.pos().' E pointer'); })
                (?&__ModifiedType__)
              )
            |
            W 4 (?&__Class__)
            |
            [0-9]
            |
            \$ (?:
                 [0DQ] (?&__Number__)
                 |
                 F (?&__Number__){2}
                 |
                 G (?&__Number__){3}
                 |
                 \$ [ABCD] (?&__DataTypeNotInArgs__)
               )
          )
        )
        (?<__ModifiedType__>
          [ABCD]
          (?:
            # multidimensional array
            Y (?&__Number__)+
          )?
          (?&__DataTypeNotInArgs__)
        )
        (?<__DataTypeInArgs__>
          (?:
            (?&__DataTypeCommon__)
            |
            # template parameter
            \? (?&__Number__)
          )
        )
        (?<__DataTypeNotInArgs__>
          (?:
            (?&__DataTypeCommon__)
            |
            \? (?&__ModifiedType__)
          )
        )
      )

      # All mangled names start with a question mark
      \?
      (?:
        # Ctors, dtors, operators etc have separate a priori defined
        # special mangled names like the very simple ?0 for constructor
        # and ?_R16789 for "RTTI Base Class Descriptor at (6,7,8,9)"
        # whatever that might mean.
        (
          \?
          ([0-9A-Z]
           |
           _(?:
              # C is for string literals, see below
              # R is RTTI, see immediately below
              [0-9ABD-QS-Z]
              |
              R0(?&__DataTypeNotInArgs__)
              |
              R1(?&__Number__){4}
              |
              R[234]
              |
              _(?:
                 E
               )
           )
          )
        )?
        (?&__Class__)

        (?:
          # Static members and normal variables
          [0-5]
            (?&__DataTypeNotInArgs__)
            [ABCD]
          |
          # Compiler-generated static
          [67]
            [ABCD]
            (?:
              @
              |
              (?&__Class__)
            )
          |
          # Non-static Methods, implicit 'this'
          [ABEFIJMNQRUV]
            [AB]
            (?{ push(@opstack, 'i '.(pos()-1).' E this'); })
            (?&__CallingConvention__)
            (?:
              @
              |
              (?&__DataTypeNotInArgs__)
            )
            (?&__ArgsZTerminated__)
          |
          # Static methods
          [CDKLST]
            (?&__CallingConvention__)
            (?:
              @
              |
              (?&__DataTypeNotInArgs__)
            )
            (?&__ArgsZTerminated__)
          |
          # Thunks
          [GHOPWX]
            ((?&__Number__))
            (?{ double_thunk($^N, pos()); })
            [AB]
            (?{ push(@opstack, 'i '.(pos()-1).' E this'); })
            (?&__CallingConvention__)
            (?:
              @
              |
              (?&__DataTypeNotInArgs__)
            )
            (?&__ArgsZTerminated__)
          |
          # Functions
          [YZ]
            (?&__CallingConvention__)
            (?:
              @
              |
              (?&__DataTypeNotInArgs__)
            )
            (?&__ArgsZTerminated__)
          |
          # Template
          \$ (?&__Identifier__) (?&__ArgsNonZTerminated__)
        )
        |
        # pooled string literals
        \?_C\@_[01](?&__Number__)(?&__32BitChecksum__)(?&__StringLiteralText__)@
      )
      /x;

    while (my $op = pop(@opstack))
      {
        # print STDERR "op=$op\n";
        my @a = split (' ', $op);
        if ($a[0] eq 'i') {
          substr($_,$a[1],0) = $a[2];
        } elsif ($a[0] eq 'r') {
          substr($_,$a[1],$a[2]) = $a[3];
        }
      }

    print;
  }
