#!/usr/bin/env python

import getopt
import os
import re
import sys
#
from srclexer import SrcLexer
from srcparser import SrcParser
from boxer import Boxer
# FIXME
from globals import *

def option_parser ():
    import optparse
    p = optparse.OptionParser ()

    p.usage = '''src2xml.py [OPTION]... SRC-FILE...'''

    examples = '''
Examples:
  src2xml.py --output-dir=. --post-process --ignore-includes zoom.src
  src2xml.py --dry-run -I svx/inc -I svx/source/dialog zoom.src
'''

    def format_examples (self):
        return examples
    
    if 'epilog' in  p.__dict__:
        p.formatter.format_epilog = format_examples
        p.epilog = examples
    else:
        p.formatter.format_description = format_examples
        p.description = examples

    p.description = '''OOo SRC To Layout XML Converter.

Convert OO.o's existing dialog resource files into XML layout files.
'''

    p.add_option ('-l', '--debug-lexer', action='store_true',
                  dest='debug_lexer', default=False,
                  help='debug lexer')

    p.add_option ('-p', '--debug-parser', action='store_true',
                  dest='debug_parser', default=False,
                  help='debug parser')

    p.add_option ('-m', '--debug-macro', action='store_true',
                  dest='debug_macro', default=False,
                  help='debug macro')

    p.add_option ('-n', '--dry-run', action='store_true',
                  dest='dry_run', default=False,
                  help='dry run')

    p.add_option ('-k', '--keep-going', action='store_true',
                  dest='keep_going', default=False,
                  help='continue after error')

    p.add_option ('-i', '--ignore-includes', action='store_true',
                  dest='ignore_includes', default=False,
                  help='ignore #include directives')

    p.add_option ('-I', '--include-dir', action='append',
                  dest='include_path',
                  default=[],
                  metavar='DIR',
                  help='append DIR to include path')

    def from_file (option, opt_str, value, parser):
        lst = getattr (parser.values, option.dest)
        lst += file (value).read ().split ('\n')
        setattr (parser.values, option.dest, lst)

    def from_path (option, opt_str, value, parser):
        lst = getattr (parser.values, option.dest)
        lst += value.split (':')
        setattr (parser.values, option.dest, lst)

    # Junk me?
    p.add_option ('--includes-from-file', action='callback', callback=from_file,
                  dest='include_path',
                  default=[],
                  type='string',
                  metavar='FILE',
                  help='append directory list from FILE to include path')

    p.add_option ('--include-path', action='callback', callback=from_path,
                  dest='include_path',
                  type='string',
                  default=[],
                  metavar='PATH',
                  help='append PATH to include path')

    p.add_option ('--only-expand-macros', action='store_true',
                  dest='only_expand_macros', default=False,
                  help='FIXME: better to say what NOT to expand?')

    p.add_option ('-o', '--output-dir', action='store',
                  dest='output_dir', default=None,
                  metavar='DIR',
                  help='Output to DIR')

    p.add_option ('-s', '--post-process', action='store_true',
                  dest='post_process', default=False,
                  help='post process output for use in Layout')

    p.add_option ('--stop-on-header', action='store_true',
                  dest='stopOnHeader', default=False,
                  help='FIXME: remove this?')

    return p


def convert (file_name, options):
    progress ("parsing %(file_name)s ..." % locals ())
    fullpath = os.path.abspath(file_name)
    if not os.path.isfile(fullpath):
        error("no such file", exit=True)

    ##options.include_path.append (os.path.dirname (fullpath))

    input = file (fullpath, 'r').read()
    lexer = SrcLexer(input, fullpath)
    lexer.expandHeaders = not options.ignore_includes
    lexer.includeDirs = options.include_path
    lexer.stopOnHeader = options.stopOnHeader
    lexer.debugMacro = options.debug_macro
#     lexer.debug = True
    if options.debug_lexer:
        lexer.debug = True
        lexer.tokenize()
        progress ("-"*68 + "\n")
        progress ("** token dump\n")
        lexer.dumpTokens()
        progress ("** end of token dump\n")
        return

    # Tokenize it using lexer
    lexer.tokenize()

    parser = SrcParser(lexer.getTokens(), lexer.getDefines())
    parser.only_expand_macros = options.only_expand_macros
    if options.debug_parser:
        parser.debug = True
        root = parser.parse()
        print root.dump()
        return

    # Parse the tokens.
    root = parser.parse()

    # Box it, and return the XML tree.
    root = Boxer(root).layout()
    output = root.dump()
    if not options.dry_run:
        progress ("\n")
    return output

def dry_one_file (file_name, options):
    try:
        str = convert(file_name, options)
        progress ("  SUCCESS\n")
    except Exception, e:
        if options.keep_going:
            progress ("  FAILED\n")
        else:
            import traceback
            print traceback.format_exc (None)
            raise e

def post_process (s):
    """Make output directly usable by layout module."""
    s = re.sub ('(</?)([a-z]+)-([a-z]+)-([a-z]+)', r'\1\2\3\4', s)
    s = re.sub ('(</?)([a-z]+)-([a-z]+)', r'\1\2\3', s)
    s = re.sub ('(<(radiobutton|(fixed(info|text)))[^>]*) text=', r'\1 label=', s)
    s = re.sub (' (height|width|x|y)="[0-9]*"', '', s)

    s = s.replace ('<modaldialog', '<modaldialog sizeable="true"')
    s = s.replace (' rid=', ' id=')
    s = s.replace (' border="true"', ' has_border="true"')
    s = s.replace (' def-button="true"', ' default="true"')
    return s

XML_HEADER = '<?xml version="1.0" encoding="UTF-8"?>\n'

def do_one_file (file_name, options):
    str = XML_HEADER
    str += convert(file_name, options)
    str += '\n'

    if options.post_process:
        str = post_process (str)
    h = sys.stdout
    if options.output_dir:
        base = os.path.basename (file_name)
        root, ext = os.path.splitext (base)
        out_name = options.output_dir + '/' + root + '.xml'
        progress ("writing %(out_name)s ..." % locals ())
        h = file (out_name, 'w')
    h.write (str)
    h.flush ()
    progress ("\n")

def main ():
    p = option_parser ()
    (options, files) = option_parser ().parse_args ()
    if not files:
        p.error ("no input files")

    for f in files:
        if options.dry_run:
            dry_one_file (f, options)
        else:
            do_one_file (f, options)
        
if __name__ == '__main__':
    main ()
