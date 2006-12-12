:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: XMLBuildListParser.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2006-12-12 16:30:19 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************


#************************************************************
# Data structure and XML parser/creator for the changeover  *
# of the current build.lst to build.xlist files             *
# programmer: Pascal Junck, Sun Microsystems GmbH           *
#************************************************************

# The current XMLBuildListParser is the second step for the changeover to XML.
# It was written to create a complex structure with more possibilities to store informations
# of the 'build.lst' files in the new XML files. The important informations were parsed by the
# 'buildlst_parser.pl' from the old files and then are (temporary) stored by the set and add
# methods in the module 'XMLBuildListParser' and 'XML::Parser'. By the API of this module it's
# possible to create the new XML 'build.xlist' files.

# If the '$Product' or the '$JobPlatform' (as also '$BuildReqPlatform') have no value,
# their value are automatically 'all'.
# It doesn't matter whether it's the set/add or the get method, that receives or sends
# these optional parameters.
# In the created XML file the default values aren't set, because of the constraints of the
# 'Document Type Definition'(DTD).
# If there is no product/platform attribute in the 'build.xlist' it means a default of 'all'!

# The important parameters are:

#     $ModuleName      =  it's the name of the current module

#     $DependencyType  =  here are the three possible scalar dependency values
#                         for all other depending modules(of the current module):
#                         'md-simple', 'md-always' and 'md-force'

#     $Products        =  which products can be used for the module dependencies
#                         and might have more different whitespace separated values
#                         e.g.'so oo' (scalar type)

#     $Dir             =  it means a string(scalar) with the current working directory,
#                         with a '/'(current directory) at the beginning of the string

#     $JobType         =  it means a job e.g. 'make'

#     $Platforms       =  in this scalar parameter might be more than one different value,
#                         like: 'wnt unx mac' and 'all'('all' includes the three values)
#                         it must be whitespace separated

#     @DependingDirs   =  a list(array) of all inner depending directories
#                         of the current working directory

#     %BuildReq        =  means a hash with build requirement pairs:
#                         'BuildReqName'(key) => 'BuildReqPlatform'(value)



##############################  begin of main   ########################################

use strict;

use XML::Parser;

package XMLBuildListParser;

# global variable for printing out all results at parsing
# if the debug variable is set to '1' it prints the results to STDOUT
my $Debug = 0;


#############################  begin of subroutines  ###################################

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;

    my $self = {};

    # no real instance data yet, might change if package is extended
    bless($self, $class);
    $$self{'error_string'} = '';
    $self->beginXMLStructure();

    return $self;
}

# VG: procedure for a better error handling
sub getError
{
   my $self = shift;
   return $$self{'error_string'};
};

########################################################################################
#         sub: loadXMLFile
#        gets: $File
#    optional: -
#     returns: -
# description: get the 'path' of the 'build.xlist'(s), load it in the 'xml parser tree'
#              and fill it in the own data_structure to make it available for the API
########################################################################################
sub loadXMLFile
{
   my $self = shift;
   my $File = shift;

   if (-f $File)
   {
      my $TreeParse = new XML::Parser(Style => 'Tree');

      my $File_ref;

      eval
      {
         $File_ref = $TreeParse->parsefile($File);
      };

      if ($@)
      {
         $@ =~ s/[\r\n]//g;
         print"ERROR: $@" if ($Debug);
         $$self{'error_string'} = 'ERROR: ' . $@ . ". Error occured while trying to parse $File";

         return 0;
      }
      else
      {
         filterXMLFile($File_ref);

         $$self{"ModuleData"} = $File_ref;

         return 1;
      }
   }
   else
   {
      $$self{'error_string'} = "ERROR: cannot find file $File";
      return 0;
   }
}

########################################################################################
#         sub: filterXMLFile
#        gets: $ArrayContent_ref
#    optional: -
#     returns: -
# description: filters all '0' and whitespace based pairs of the XML file
#              -> all spaces, tabs and new lines
########################################################################################
sub filterXMLFile
{
   my $ArrayContent_ref = shift;

   # get the number of elements of the array_ref
   my $Count = getContentCount($ArrayContent_ref);

   for (my $i = 0; $i < $Count;)
   {
      # get each content pair
      my $Content_ref = getContent($ArrayContent_ref, $i);

      # in each content pair the first element is either
      # a tag name or the value '0'
      my $FirstContent = getTagName($Content_ref);

      # we need the second part of the content pair to check
      # which value is inside
      my $SecondContent = getSecondContent($Content_ref);

      my $tempArray_ref = "";
      if (($FirstContent eq "task") or ($FirstContent eq "depend"))
      {
        my $dir = $Content_ref->[1]->[0]->{dir};
        $dir =~ s/\/$//;
        $Content_ref->[1]->[0]->{dir} = $dir;
        $i++;
        filterXMLFile($Content_ref);
      }
      elsif ($FirstContent eq "0")
      {
         # only if there is in the first part a '0' and in the
         # second part are whitespaces...
         if ($SecondContent =~ /\s+/)
         {
            # ...make a ref at this position
            $tempArray_ref = @$ArrayContent_ref[1];

            # and delete this element pair
            removeContent($tempArray_ref, $i);

            # now we have one element pair fewer
            $Count--;
         }
         else
         {
            # is there a '0' but in the second part not a whitespace,
            # increase 'i' by 1
            $i++;
         }
      }
      else
      {
         # if it's a tag name, increase 'i' by one and call recursive
         # the 'filterXMLFile' with the 'content ref'
         $i++;

         # look further after the '0' and whitespace content
         filterXMLFile($Content_ref);
      }
   }
}

########################################################################################
#         sub: removeContent
#        gets: $File_ref, $Count
#    optional: -
#     returns: -
# description: removes the '0' and the whitespace based pairs of the XML structure
#              whitespace could be: tabs, new lines and whitespace itself
########################################################################################
sub removeContent
{
   my $tempArray_ref = shift;
   my $i = shift;

   my $Start = (2*$i) + 1;

   splice(@$tempArray_ref, $Start, 2);
}

########################################################################################
#         sub: beginXMLStructure
#        gets: $ModuleData_ref
#    optional: -
#     returns: -
# description: create a new beginning of the XML file structure
########################################################################################
sub beginXMLStructure
{
   my $self = shift;

   # global variable for the complete filled data structure
   my $ModuleData_ref = createTag("build-list", {});

   $$self{"ModuleData"} = $ModuleData_ref;
}

########################################################################################
#         sub: insertContent
#        gets: $HigherLevelTag_ref, $currentTag_ref, $Pos
#    optional: -
#     returns: -
# description: insert a content at the right (alphabetical sorted) position
########################################################################################
sub insertContent
{
   my $HigherLevelTag_ref = shift;
   my $currentTag_ref = shift;
   my $Pos = shift;

   my $Array_ref = $HigherLevelTag_ref->[1];

   $Pos = ($Pos*2)+1;

   splice(@$Array_ref, $Pos, 0, @$currentTag_ref);
}

########################################################################################
#         sub: saveXMLFile
#        gets: $Path
#    optional: -
#     returns: -
# description: creates a XML file of the whole data structure
########################################################################################
sub saveXMLFile
{
   my $self = shift;
   my $Path = shift;

   my $ModuleData_ref = $$self{"ModuleData"};

   # open the filehandle 'CREATE_XML' for creating the XML files
   open (SAVE_XML, ">".$Path)
   or die "Error. Open the file <build.xlist> wasn't successful!\n\n";

   select SAVE_XML;

   print"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
   print"<!DOCTYPE build-list SYSTEM \"build_xlist.dtd\">\n";
   print"<!--\n"
       ."***************************************************************************\n"
       ."*                                                                          \n"
       ."*   OpenOffice.org - a multi-platform office productivity suite            \n"
       ."*                                                                          \n"
       ."*   \$RCSfile: XMLBuildListParser.pm,v $                                  \n"
       ."*                                                                          \n"
       ."*   \$Revision: 1.2 $                                                 \n"
       ."*                                                                          \n"
       ."*   last change: \$Author: kz $ \$Date: 2006-12-12 16:30:19 $           \n"
       ."*                                                                          \n"
       ."*   The Contents of this file are made available subject to                \n"
       ."*   the terms of GNU Lesser General Public License Version 2.1.            \n"
       ."*                                                                          \n"
       ."*                                                                          \n"
       ."*     GNU Lesser General Public License Version 2.1                        \n"
       ."*     =============================================                        \n"
       ."*     Copyright 2005 by Sun Microsystems, Inc.                             \n"
       ."*     901 San Antonio Road, Palo Alto, CA 94303, USA                       \n"
       ."*                                                                          \n"
       ."*     This library is free software; you can redistribute it and/or        \n"
       ."*     modify it under the terms of the GNU Lesser General Public           \n"
       ."*     License version 2.1, as published by the Free Software Foundation.   \n"
       ."*                                                                          \n"
       ."*     This library is distributed in the hope that it will be useful,      \n"
       ."*     but WITHOUT ANY WARRANTY; without even the implied warranty of       \n"
       ."*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    \n"
       ."*     Lesser General Public License for more details.                      \n"
       ."*                                                                          \n"
       ."*     You should have received a copy of the GNU Lesser General Public     \n"
       ."*     License along with this library; if not, write to the Free Software  \n"
       ."*     Foundation, Inc., 59 Temple Place, Suite 330, Boston,                \n"
       ."*     MA  02111-1307  USA                                                  \n"
       ."*                                                                          \n"
       ."***************************************************************************\n"
       ."-->\n";

   printTag($ModuleData_ref);

   # flush the buffer
   # it's the same like the both lines below here, but we make it manually without module 'IO'
   # use IO::Handle;
   # SAVE_XML -> autoflush(1);
   $| = 1;

   select STDOUT;

   close (SAVE_XML);

}

########################################################################################
#         sub: printTag
#        gets: $Tag_ref
#    optional: -
#     returns: -
# description: prints out each tag of the existing data structure (XML tree)
########################################################################################
sub printTag
{
   my $Tag_ref = shift;

   # the first time we call this function the 'pos counter' has the default value of '0',
   # because it is for the begin tag -> it shouldn't have a distance to the left side
   my $PosCounter = shift || 0;

   # makes it possible that each tag has a specified distance from the left sided margin
   my $Distance = " " x $PosCounter;

   # it's possible that the content is a text(case 1) between a begin and end tag
   # or a tag name(case 2)
   # case 1: (    0      , "Text between the begin and end tag" )
   my $TagName = getTagName($Tag_ref);

   if ($TagName eq "0")
   {
        print"$Tag_ref->[1]";
   }
   # case 2: ( "TagName" , (...) )
   else
   {
      # open the tag
      # only a new line if it is not the first tag
      print"\n" if ($PosCounter != 0);
      print $Distance."<".$TagName;

      my $Attributes_ref  = getAttributesRef($Tag_ref);

      # print all sorted attributes of this tag
      foreach my $Attribute (sort keys %$Attributes_ref)
      {
         my $value = $$Attributes_ref{$Attribute};
         print" $Attribute=\"$value\"";
      }

      # get the number of elements of the array
      my $ContentCount = getContentCount($Tag_ref);

      # close the tag
      # if it is an empty tag create a '/' before we close it with '>'
      print"/" if ($ContentCount == 0);
      print">";

      # get and print all tags recursive
      for (my $i = 0; $i < $ContentCount; $i++)
      {
         # here we get the content of the current tag,
         # we rise the 'pos counter' that each level of tags
         # has a specified distance from the left side
         printTag(getContent($Tag_ref, $i), $PosCounter + 2);
      }

      # check that the tag has a text between the begin and end tag
      # first: had the tag further inner tags?
      if ($ContentCount > 0)
      {
         my $Content_ref = getContent($Tag_ref, 0);
         my $TagName = getTagName($Tag_ref);
         my $TagContent = getTagName($Content_ref);

         # if a tag has a begin tag and a following text, create the end tag behind the text
         print"</$TagName>" if ($TagContent eq "0");

         print"\n".$Distance."</$TagName>" if ($TagContent ne "0");
      }
   }
}

########################################################################################
#         sub: getTagName
#        gets: $Content_ref
#    optional: -
#     returns: $TagName
# description: gets a reference of an array and returns the tag name at position '0'
########################################################################################
sub getTagName
{
   my $Content_ref = shift;

   my $TagName = $$Content_ref[0];

   return $TagName;
}

########################################################################################
#         sub: getSecondContent
#        gets: $Content_ref
#    optional: -
#     returns: $SecondContent
# description: gets a reference of an array and returns the second value(index '1')
########################################################################################
sub getSecondContent
{
   my $Content_ref = shift;

   my $SecondContent = $$Content_ref[1];

   return $SecondContent;
}

########################################################################################
#         sub: createSortKey
#        gets: $DependencyType, $DepModuleName
#    optional: -
#     returns: $SortKey
# description: creates a key with a number(depending of the dependency type)
#              and a string and returns it
########################################################################################
sub createSortKey
{
   my $DependencyType = shift;
   my $DepModuleName = shift;

   my $DepTypeAsValue = 0;

   if ($DependencyType eq "md-simple")
   {
      $DepTypeAsValue = 1;
   }
   elsif ($DependencyType eq "md-always")
   {
      $DepTypeAsValue = 2;
   }
   elsif ($DependencyType eq "md-force")
   {
      $DepTypeAsValue = 3;
   }

   my $SortKey = $DepTypeAsValue.$DepModuleName;

   return $SortKey;
}

########################################################################################
#         sub: searchTag
#        gets: $TagName, $Tag_ref
#    optional: -
#     returns: $TagValues_ref
# description: gets a reference and goes in the lower level tag,
#              looks after the specified tag name and returns the reference
#              of itself and the neighbour element
########################################################################################
sub searchTag
{
   my $TagName = shift;
   my $Tag_ref = shift;

   my $TagSerie_ref = $Tag_ref->[1];

   # (all elements of the array) - 1 => highest index of the array
   my $IndexEnd = scalar(@$TagSerie_ref)-1;

   my $TagValues_ref = undef;

   for (my $i = 1; $i <= $IndexEnd; $i += 2)
   {
      if ($TagSerie_ref->[$i] eq $TagName)
      {
         @$TagValues_ref = @$TagSerie_ref[$i..$i+1];

         last;
      }
   }

   return $TagValues_ref;
}

########################################################################################
#         sub: createTag
#        gets: $TagName, $Attribute_ref
#    optional: -
#     returns: $Tag_ref
# description: creates an anonymous array with a tag name and the reference of its
#              attributes; then returns the reference of this array
########################################################################################
sub createTag
{
   my $TagName = shift;
   my $Attribute_ref = shift;

   my $Tag_ref = [$TagName, [$Attribute_ref]];

   return $Tag_ref;
}

########################################################################################
#         sub: createText
#        gets: $Text
#    optional: -
#     returns: $TagText_ref
# description: creates an anonymous array(which contains a '0' and a string)
#              for the data structure and returns a reference of it
########################################################################################
sub createText
{
   my $Text = shift;

   my $TagText_ref = [0, $Text];

   return $TagText_ref;
}

########################################################################################
#         sub: addContent
#        gets: $Tag_ref, $NextInfos_ref
#    optional: -
#     returns: -
# description: creates and adds a tag with its following content
########################################################################################
sub addContent
{
   # '$NextInfos_ref' has two infos:
   # either:  '0' and a string                          -> [Tag, [{}, 0     ,  string ]]
   #     or:  'tag' and a ref of a anonymous array      -> [Tag, [{}, newTag, [.....] ]]
   my $Tag_ref = shift;
   my $NextInfos_ref = shift;

   my $SecondPart_ref = $Tag_ref->[1];

   # '$Tag_ref' gets now the right structure:
   # e.g.  ['tag1', [{}, 0, "text of tag1" ...]...]
   # or    ['tag1', [{}, 'tag2', [...]     ...]...]
   push(@$SecondPart_ref, @$NextInfos_ref);
}

########################################################################################
#         sub: getContent
#        gets: $Tag_ref, $i
#    optional: -
#     returns: \@Content (ref of array 'content')
# description: creates and returns a reference of an array with two elements,
#              which are a part of the array of the current tag
########################################################################################
sub getContent
{
   my $Tag_ref = shift;
   my $i = shift;

   # e.g.
   #          [ "tag 1",  [  {} ,  0 , "string", "tag 2", [{}..], "tag 3", [{}..], ..] ..]
   # $Tag_ref [  [0]     [1]                                                           ..]
   # $Content_ref         [ [0]   [1]    [2]       [3]    [4]      [5]     [6]     ..]

   my $Content_ref = $$Tag_ref[1];

   # '$start' contains: elements with the index 1, 3, 5 ...(and so on)
   #  '$end'  contains: elements with the index 2, 4, 6 ...(and so on)
   my $start = (2 * $i) + 1;
   my $end = $start + 1;

   my @Content = @$Content_ref[$start..$end];

   return \@Content;
}

########################################################################################
#         sub: getAttribute
#        gets: $Tag_ref, $AttributeName
#    optional: -
#     returns: $Attribute
# description: finds and returns an attribute as a string
########################################################################################
sub getAttribute
{
   my $Tag_ref = shift;
   my $AttributeName = shift;

   #          [ "tag1",  [   { attribute1 = "..",    attribute2 = ".." } , "tag2"...  ]  ]
   # Tag_ref  [  [0]    [1]                                                           ]  ]
   #                    [  [0]->'attribute1'    , [0]->'attribute2'      ,   [1] ...  ]  ]

   # get the scalar of the value of the 'AttributeName' in the anonymous hash
   my $Attribute = $Tag_ref->[1]->[0]->{$AttributeName};

   return $Attribute;
}

########################################################################################
#         sub: getAttributeRef
#        gets: $Tag_ref
#    optional: -
#     returns: $Attribute_ref
# description: finds and returns the reference of the hash which contains the attributes
########################################################################################
sub getAttributesRef
{
   my $Tag_ref = shift;

   #         [ "tag1",  [   {  attribute1 = "..",    attribute2 = ".." } , "tag2"... ]  ]
   # Tag_ref [  [0]   [1]                                                            ]  ]
   #                   [   [0]                                           ,  [1]      ]  ]

   # get the reference of the anonymous hash
   my $Attribute_ref = $Tag_ref->[1]->[0];

   return $Attribute_ref;
}

########################################################################################
#         sub: getContentCount
#        gets: $Tag_ref
#    optional: -
#     returns: $IndexValue
# description: returns the sum of elements pairs of the array (less the anonymous hash)
########################################################################################
sub getContentCount
{
   my $Tag_ref = shift;

   # get the content array (address of the inner array)
   my $Content_ref = $$Tag_ref[1];

   # get the number of the element pairs of the array (without the anonymous hash)
   my $IndexValue = scalar(@$Content_ref);
   $IndexValue = ($IndexValue-1)/2;

   return $IndexValue;
}

########################################################################################
#         sub: getIterationData
#        gets: $TagName
#    optional: -
#     returns: $IndexValue, $TagDepend_ref
# description: gets a tag name and returns the sum of elements pairs of the array
#             (less the anonymous hash) and the reference of the anonymous array
########################################################################################
sub getIterationData
{
   my $self = shift;
   my $TagName = shift;

   my $ModuleData_ref = $$self{"ModuleData"};

   my $TagDepend_ref = searchTag($TagName, $ModuleData_ref);
   my $IndexValue = getContentCount($TagDepend_ref);

   return ($IndexValue, $TagDepend_ref);
}

########################################################################################
#         sub: printErrorMessage
#        gets: $BuildReqPlatforms or $Platforms or $Products
#    optional: -
#     returns: -
# description: it's an error, if a platform or a product content (e.g.'so oo')
#              has at least one valid value and also the value 'all',
#              because 'all' includes all other possible values
########################################################################################
sub printErrorMessage
{
   my $self = shift;
   my $Content = shift;

   my $ModuleData_ref = $$self{"ModuleData"};

   my $Module_ref = searchTag("module-name", $ModuleData_ref);
   my $ModuleContent_ref = getContent($Module_ref, 0);
   my $Module = @$ModuleContent_ref[1];

   print"Error in module <$Module> in Content <$Content>!\n";
   print"The value 'all' includes all currently existing valid values.\n\n";
}

########################################################################################
#         sub: adjustRedundancy
#        gets: $Task_ref, $JobPlatform, $BuildReq_ref
#    optional: -
#     returns: -
# description: the tag <task> (it means 'job') should be created with no redundant infos,
#              like: <task dir="/uno" platform="unx"> ...
#                    <task dir="/uno" platform="wnt"> ...
#                       <build-requirement name="test10" platform="wnt">
#
#              it should be: <task dir="/uno" platform="unx wnt">
#                               <build-requirement name="test10" platform="wnt">
########################################################################################
sub adjustRedundancy
{
   my $Task_ref = shift;
   my $JobPlatform = shift;
   my $BuildReq_ref = shift;

   my %sortedJobPlatforms = ();
   my @JobPlatforms = ();
   my $PlatformContent = "";

   # get the 'task platforms' in one content
   my $Attributes_ref = getAttributesRef($Task_ref);

   # get the existing 'task platform'
   my $existingPlatform = getAttribute($Task_ref, "platform");
   $existingPlatform = "all" if (!($existingPlatform));

   if ( ($existingPlatform ne "all") && ($JobPlatform ne "all") )
   {
      # get the sorted platform content
      $sortedJobPlatforms{$JobPlatform} = "";
      $sortedJobPlatforms{$existingPlatform} = "";

      @JobPlatforms = sort keys %sortedJobPlatforms;

      $PlatformContent = join " ", @JobPlatforms;

      $$Attributes_ref{"platform"} = $PlatformContent;
   }
   elsif ( ($existingPlatform ne "all") && ($JobPlatform eq "all") )
   {
      delete $$Attributes_ref{"platform"};
   }

   if ($BuildReq_ref)
   {
      my $JobType_ref = getContent($Task_ref, 0);

      # if it exists add the 'build requirements' at the
      # previous(with the same directory as the current) <task> tag
      addBuildReq($JobType_ref, $BuildReq_ref);
   }
}

########################################################################################
#         sub: addBuildReq
#        gets: $JobTypeInfos_ref, $BuildReq_ref
#    optional: -
#     returns: -
# description: add the 'build requirements' to the existing data structure
#              case 1: if the 'job directory' is not redundant
#              case 2: if it is redundant create it to the first existing 'job'
########################################################################################
sub addBuildReq
{
   my $JobTypeInfos_ref = shift;
   my $BuildReq_ref = shift;

   my @sortedBuildReqPlatforms = ();

   foreach my $BuildReqName (sort keys %$BuildReq_ref)
   {
      my $BuildReqPlatforms = $$BuildReq_ref{$BuildReqName};

      $BuildReqPlatforms = "all" if (!($BuildReqPlatforms));

      my $Attributes_ref = {"name" => "$BuildReqName"};

      # it's wrong, if in a platform content(e.g. "unx wnt") are at least
      # one or more platform(s) and within a 'all' term,
      # because 'all' is default and means it includes all other possible values!
      if ( ($BuildReqPlatforms ne "all") && ($BuildReqPlatforms =~ /\ball\b/) )
      {
         printErrorMessage($BuildReqPlatforms);
      }
      elsif (!($BuildReqPlatforms =~ /\ball\b/))
      {
         @sortedBuildReqPlatforms = sort(split(/\s+/, $BuildReqPlatforms));
         $BuildReqPlatforms = join " ", @sortedBuildReqPlatforms;

         $$Attributes_ref{"platform"} = "$BuildReqPlatforms";
      }

      # create the tag <build-requirement>
      my $BuildReqInfos_ref = createTag("build-requirement", $Attributes_ref);

      # append the <build-requirement> tag to the <'$JobType'> tag
      addContent($JobTypeInfos_ref, $BuildReqInfos_ref);
   }
}

########################################################################################
#         sub: checkJobRedundancy
#        gets: $Task_ref, $JobType, $DependingDirs_ref, $JobPlatform, $BuildReq_ref
#    optional: -
#     returns: $LineIsRedundant
# description: checks whether the values of the 'job' line are redundant, like:
#              'job dir', 'job'(e.g.: make) and 'depending dirs'
########################################################################################
sub checkJobRedundancy
{
   my $Task_ref = shift;
   my $JobType = shift;
   my $DependingDirs_ref = shift;
   my $JobPlatform = shift;
   my $BuildReq_ref = shift;

   my $LineIsRedundant = 0;


   # get the ref of the existing 'depending directories'
   # if they also equal with the current 'depending directories',
   # make one tag instead of two, which differences only in the platform
   # (and the 'build requirement', if it exists)
   my $JobType_ref = getContent($Task_ref, 0);
   my $JobName = getTagName($JobType_ref);

   # get the existing 'task platform'
   my $existingPlatform = getAttribute($Task_ref, "platform");

   # are the jobs equal?
   if ($JobType eq $JobName)
   {
      my @existingDepDirs = ();

      my $IndexEnd = getContentCount($JobType_ref);

      # get all existing 'depending dirs' of this redundant 'job'
      for (my $j = 0; $j < $IndexEnd; $j++)
      {
         my $Content_ref = getContent($JobType_ref, $j);

         my $TagName = getTagName($Content_ref);

         # create an array of the 'depending directories'
         if ($TagName eq "depend")
         {
            my $DepDir = getAttribute($Content_ref, "depend");

            push(@existingDepDirs, $DepDir);
         }
      }

      # if now the current 'depending dirs' equal with the existing,
      # we know that is redundant and have to create only one instead
      # of two tags, e.g.
      #            before: <task dir="/uno" platform="unx">...
      #                    <task dir="/uno" platform="wnt"> ...
      #                       <build-requirement name="test10" platform="wnt">
      #
      #      it should be: <task dir="/uno" platform="unx wnt">
      #                       <build-requirement name="test10" platform="wnt">
      if (@$DependingDirs_ref eq @existingDepDirs)
      {
         $LineIsRedundant = 1;

         # check redundant directories and create no redundant 'task dirs'
         adjustRedundancy($Task_ref, $JobPlatform, $BuildReq_ref);
      }
   }

   return $LineIsRedundant;
}

########################################################################################
#         sub: existsTag
#        gets: $TagName
#    optional: -
#     returns: $TagExists_ref
# description: checks whether that a tag exists and returns the ref of the content
########################################################################################
sub existsTag
{
   my $self = shift;
   my $TagName = shift;

   my $TagExists_ref = undef;

   my $ModuleData_ref = $$self{"ModuleData"};

   # check whether that the <module-depend> tag exists
   $TagExists_ref = searchTag($TagName, $ModuleData_ref);

   return $TagExists_ref;
}

########################################################################################
#                           API - (internal) 'set/add' methods                         #
########################################################################################

########################################################################################
#         sub: setModuleName
#        gets: $ModuleName
#    optional: -
#     returns: -
# description: gets the name of the current module and set it at the right position
#              in the data structure
########################################################################################
sub setModuleName
{
   my $self = shift;
   my $ModuleName = shift;

   my $ModuleData_ref = $$self{"ModuleData"};

   my $Tag_ref = createTag("module-name", {});
   my $TagText_ref = createText($ModuleName);

   addContent($Tag_ref, $TagText_ref);

   addContent($ModuleData_ref, $Tag_ref);
}

########################################################################################
#         sub: addModuleDependencies
#        gets: $ModuleName, $DependencyType, $Products
#    optional: $Products(default: 'all' -> includes currently 'so' and 'oo')
#              but the default should not set in the data structure,
#              it's only a 'Document Type Definition' based term
#     returns: -
# description: add the module dependencies and their attributes into the data structure
########################################################################################
sub addModuleDependencies
{
   my $self = shift;
   my $ModuleName = shift;
   my $DependencyType = shift;
   my $Products = shift || "all";

   my $ModuleData_ref = $$self{"ModuleData"};

   my @sortedProducts = ();

   # change all possible upper cases to lower cases
   $Products =~ s/($Products)/\L$Products/;

   # before we add the module dependencies, we have to prove that the <module-depend> tag was set
   # because this tag must be set once before the module dependency tags begin
   my $ModuleDepend_ref = searchTag("module-depend", $ModuleData_ref);

   # if it doesn't exist, create this tag '<module-depend>'
   if (!($ModuleDepend_ref))
   {
      $ModuleDepend_ref = createTag("module-depend", {});

      # add it to the global data structure
      addContent($ModuleData_ref, $ModuleDepend_ref);
   }

   my $Attributes_ref = {"module" => "$ModuleName"};

   # it's wrong, if in a product content(e.g. "so") are at least
   # one or more product(s) and within a 'all' term,
   # because 'all' is default and means it includes all other possible values!
   if ( ($Products ne "all") && ($Products =~ /\ball\b/) )
   {
      printErrorMessage($Products);
   }
   elsif (!($Products =~ /\ball\b/))
   {
      @sortedProducts = sort(split(/\s+/ ,$Products));
      $Products = join " ", @sortedProducts;

      $$Attributes_ref{"product"} = "$Products";
   }

   my $ModuleDependenciesInfos_ref = createTag("$DependencyType", $Attributes_ref);

   my $currentKey = createSortKey($DependencyType, $ModuleName);

   # search and get the position in which we have to insert the current 'module depend name'
   # at first get the current 'module depend name'
   my $currentName = getAttribute($ModuleDependenciesInfos_ref, "module");

   # get the information about the number of 'Contents'(= elements) of the array
   my $ContentCount = getContentCount($ModuleDepend_ref);

   # we have to sort the serie of the 'name' contents,
   # therefore we need a 'Pos'(position) of the array in which we want to sort in the 'name' content
   my $Pos = 0;

   # and we need a control variable 'isInsert'
   # that we won't add the 'name' and the content more than one time
   my $isInsert = 0;

   for (my $i = 0; $i < $ContentCount; $i++)
   {
      # get each 'Content' of the array = ('task', ARRAY(...))
      my $Content_ref = getContent($ModuleDepend_ref, $i);
      my $TagName = getTagName($Content_ref);

      # get the existing 'task dir' to compare it with the current 'task dir'
      my $existingName = getAttribute($Content_ref, "module");

      my $existingKey = createSortKey(getTagName($Content_ref), $existingName);

      # compare both dirs...
      # only if the 'current dir' is lower than a 'existing dir'
      # insert it in the data structure
      if ($currentKey lt $existingKey)
      {
         $Pos = $i;

         insertContent($ModuleDepend_ref, $ModuleDependenciesInfos_ref, $Pos);

         $isInsert = 1;

         last;
      }
   }
   # only if the 'current name' is greater (or equal) than the other 'existing names'
   # insert it at the end of the data structure
   addContent($ModuleDepend_ref, $ModuleDependenciesInfos_ref) if ($isInsert == 0);
}

########################################################################################
#         sub: addJob
#        gets: $Dir, $JobType, $JobPlatform, $DependingDirs_ref, $BuildReq_ref,
#              $JobPlatform, $DependingDirs_ref, $BuildReq_ref
#    optional: $JobPlatform(default: 'all' -> includes all other possible values),
#              $DependingDirs_ref, $BuildReq_ref
#     returns: -
# description: add the infos about a job from the old build lists(by ascii parser) and
#              sort it in the data structure
########################################################################################
sub addJob
{
   my $self = shift;
   my $Dir = shift;
   my $JobType = shift;
   my $JobPlatform = shift || "all";
   my $DependingDirs_ref = shift;
   my $BuildReq_ref = shift;

   my $ModuleData_ref = $$self{"ModuleData"};

   # before we add the "build" tag, we have to prove that the <build> tag was set
   # because this tag must be set once before the job tag(s) follows
   my $buildTag_ref = searchTag("build", $ModuleData_ref);

   # if it doesn't exist, create the tag '<build>'
   if (!($buildTag_ref))
   {
      # If the tag wasn't found, create it
      $buildTag_ref = createTag("build", {});

      # add it to the global data structure
      addContent($ModuleData_ref, $buildTag_ref);
   }

   my $Attributes_ref = {"dir" => "$Dir"};

   # it's wrong, if a 'job platform' content(e.g. "unx wnt") has at least
   # one or more 'job platform(s)' and an 'all' term,
   # because 'all' is default and means it includes all other possible values
   if ( ($JobPlatform ne "all") && ($JobPlatform =~ /\ball\b/) )
   {
      printErrorMessage($JobPlatform);
   }
   elsif (!($JobPlatform =~ /\ball\b/))
   {
      my @sortedPlatforms = sort(split /\s+/, $JobPlatform);
      $JobPlatform = join " ", @sortedPlatforms;

      $$Attributes_ref{"platform"} = "$JobPlatform";
   }

   # create the tags: <task>, <make> and (if it exists)...
   #                  <depend> and/or <build-requirement>
   my $taskInfos_ref = createTag("task", $Attributes_ref);

   # search and get the position in which we have to insert the current task
   # at first get the current 'task directory'
   my $currentDir = getAttribute($taskInfos_ref, "dir");

   # get the information about the number of 'Contents'(= elements) of the array
   my $IndexValue = getContentCount($buildTag_ref);

   # we have to sort the serie of the 'task contents',
   # therefore we need a '$pos'(position) of the array in which we want to sort in the 'task content'
   my $Pos = 0;

   # and we need a control variable 'isInsert'
   # that we won't add the 'task content' more than one time
   my $isInsert = 0;

   # control variable for the redundancy check
   my $LineIsRedundant = 0;

   # go in the array of the corresponding <build> tag element
   for (my $i = 0; $i < $IndexValue; $i++)
   {
      # get each content of the <build> tag => ('task1', ARRAY1(...), task2...)
      my $Task_ref = getContent($buildTag_ref, $i);

      # get the existing 'task dir' to compare it with the current 'task dir'
      my $existingDir = getAttribute($Task_ref, "dir");

      # is the 'job dir' redundant?
      if ($currentDir eq $existingDir)
      {
         $LineIsRedundant = checkJobRedundancy($Task_ref, $JobType, $DependingDirs_ref, $JobPlatform, $BuildReq_ref);
      }

      # if it's not a redundant line, compare both dirs:
      # only if the 'current dir' is lower than an 'existing dir'
      # insert it in the data structure
      if ( ($LineIsRedundant == 0) && ($currentDir lt $existingDir) )
      {
         $Pos = $i;

         insertContent($buildTag_ref, $taskInfos_ref, $Pos);

         $isInsert = 1;

         last;
      }
   }

   # only if the 'current dir' is greater (or equal) than the other 'existing dirs'
   # and it is not redundant insert it at the end of the data structure
   if ( ($isInsert == 0) && ($LineIsRedundant == 0) )
   {
      addContent($buildTag_ref, $taskInfos_ref);
   }

   if ($LineIsRedundant == 0)
   {
      # create the <'$JobType'> tag
      my $JobTypeInfos_ref = createTag($JobType, {});

      # append the <'$JobType'> tag to the <task> tag
      addContent($taskInfos_ref, $JobTypeInfos_ref);

      # before we add the "depend" infos
      # we have to get the alphabetical sorted 'Depending Directories'
      @$DependingDirs_ref = sort(@$DependingDirs_ref) if ($DependingDirs_ref);

      foreach my $DependDir (@$DependingDirs_ref)
      {
         my $DependInfos_ref = createTag("depend", {"dir" => "$DependDir"});

         # append the <depend> tag to the <'$JobType'> tag
         addContent($JobTypeInfos_ref, $DependInfos_ref);
      }

      # if a 'build requirement' exists, create the tag <build-requirement>
      if ($BuildReq_ref)
      {
         addBuildReq($JobTypeInfos_ref, $BuildReq_ref);
      }
   }
}

########################################################################################
#                           end of (internal) 'set/add' methods                        #
########################################################################################


########################################################################################
#                             API - (external) 'get' methods                           #
########################################################################################

########################################################################################
#         sub: getModuleDependencies
#        gets: $Product, $DependencyType
#    optional: $Product(default: 'all', means all currently used valid values),
#              $DependencyType(default: 'md-simple', 'md-always' and 'md-force')
#     returns: @ModuleDependencies
# description: gets a ref of an array (with the products) and creates and
#              returns an array with the sorted depending modules
########################################################################################
sub getModuleName {
   my $self = shift;
   if ($self->existsTag("module-name")) {
       my ($IndexValue, $ModuleDepend_ref) = $self->getIterationData("module-name");
       return $$ModuleDepend_ref[1][2];
   };
   return "";

};
sub getModuleDependencies
{
   my $self = shift;
   my $Products_ref = shift;
   my $DependencyType = shift || "all";

   push(@$Products_ref, "all") if (!scalar @$Products_ref);

   my $Product = "";
   my %tempModuleDeps = ();
   my @ModuleDependencies = ();

   my $ModuleData_ref = $$self{"ModuleData"};

   # check whether that the <module-depend> tag exists
   if ($self->existsTag("module-depend"))
   {
      # change all possible upper cases to lower cases
      $DependencyType =~ s/($DependencyType)/\L$DependencyType/ if ($DependencyType ne "all");

      foreach $Product (@$Products_ref)
      {
         # change all possible upper cases to lower cases
         $Product =~ s/($Product)/\L$Product/;

         my $ProductContent = undef;
         my $ModuleDependencyName = "";

         # get the number of elements and the ref of the <module-depend> tag
         my ($IndexValue, $ModuleDepend_ref) = $self->getIterationData("module-depend");

         for (my $i = 0; $i < $IndexValue; $i++)
         {
            my $Content_ref = getContent($ModuleDepend_ref, $i);

            my $ModuleDependencyName = getAttribute($Content_ref, "module");

            # get the name of each existing tag
            my $TagName = getTagName($Content_ref);

            $ProductContent = getAttribute($Content_ref, "product");

            # if the attribute 'product' wasn't set in the internal data structure,
            # it means the default of 'all' is valid and includes all other possible values
            $ProductContent = "all" if (!($ProductContent));

            if ($Product ne "all")
            {
               if ($DependencyType ne "all")
               {
                  # if the caller wants all (e.g.)'so' product based dependency types,
                  # we must get the 'so' and the 'all' matching products
                  # because 'all' matches also the product 'so'
                  if ( ($DependencyType eq $TagName) &&
                       ((($ProductContent eq "all") || $ProductContent =~ /\b($Product)\b/)) )
                  {
                     $tempModuleDeps{$ModuleDependencyName} = "";

                     print"ModuleDeps (Product != 'all' && DepType != 'all') = <$ModuleDependencyName>\n" if ($Debug);
                  }
               }
               # we get from the caller only the 'product' parameter,
               # 'dependency type' is now 'all'(default) and includes all possible values
               elsif ( ($ProductContent =~ /\b($Product)\b/) || ($ProductContent eq "all") )
               {
                  $tempModuleDeps{$ModuleDependencyName} = "";

                  print"ModuleDeps (Product != 'all' && DepType = 'all') = <$ModuleDependencyName>\n" if ($Debug);
               }
            }
            # now the product is 'all' and we only need to check the 'dependency type'
            elsif ($DependencyType ne "all")
            {
               if ($DependencyType eq $TagName)
               {
                  $tempModuleDeps{$ModuleDependencyName} = "";

                  print"ModuleDeps (Product = 'all' && DepType != 'all') = <$ModuleDependencyName>\n" if ($Debug);
               }
            }
            else
            {
               $tempModuleDeps{$ModuleDependencyName} = "";

               print"ModuleDeps (Product = 'all' && DepType = 'all') = <$ModuleDependencyName>\n" if ($Debug);
            }
         }
      }

      @ModuleDependencies = sort keys %tempModuleDeps;
   }

   print"ModuleDependencies            =  <@ModuleDependencies>\n" if ($Debug);

   return @ModuleDependencies;
}

########################################################################################
#         sub: getModuleDepType
#        gets: $DepModuleName
#    optional: -
#     returns: $DependencyType
# description: gets a module name and returns the dependency type of it
########################################################################################
sub getModuleDepType
{
   my $self = shift;
   my $DepModuleName = shift;

   my $DependencyType = "";

   my $ModuleData_ref = $$self{"ModuleData"};

   # check whether that the <module-depend> tag exists
   if ($self->existsTag("module-depend"))
   {
      # change all possible upper cases to lower cases
      $DepModuleName =~ s/($DepModuleName)/\L$DepModuleName/;

      my ($IndexValue, $ModuleDepend_ref) = $self->getIterationData("module-depend");

      for (my $i = 0; $i < $IndexValue; $i++)
      {
         my $Content_ref = getContent($ModuleDepend_ref, $i);
         my $existingModuleName = getAttribute($Content_ref, "module");

         if ($DepModuleName eq $existingModuleName)
         {
            $DependencyType = getTagName($Content_ref);
            last;
         }
      }
   }

   print"DependencyType                =  <$DependencyType>\n" if ($Debug);

   return $DependencyType;
}

########################################################################################
#         sub: getModuleProducts
#        gets: $DepModuleName
#    optional: -
#     returns: @ModuleProducts
# description: gets a module name and returns the associated products
########################################################################################
sub getModuleProducts
{
   my $self = shift;
   my $DepModuleName = shift;

   my @ModuleProducts = ();

   my $ModuleData_ref = $$self{"ModuleData"};

   # check whether that the <module-depend> tag exists
   if ($self->existsTag("module-depend"))
   {
      # change all possible upper cases to lower cases
      $DepModuleName =~ s/($DepModuleName)/\L$DepModuleName/;

      my $ProductContent = undef;

      my ($IndexValue, $ModuleDepend_ref) = $self->getIterationData("module-depend");

      for (my $i = 0; $i < $IndexValue; $i++)
      {
         my $Content_ref = getContent($ModuleDepend_ref, $i);

         my $existingModuleName = getAttribute($Content_ref, "module");

         # if the attribute 'product' wasn't set in the internal data structure,
         # it means the default of 'all' is valid and includes all other possible values
         $ProductContent = getAttribute($Content_ref, "product");

         if ($DepModuleName eq $existingModuleName)
         {
            $ProductContent = "all" if (!($ProductContent));

            @ModuleProducts = split /\s+/, $ProductContent;

            last;
         }
      }
   }

   print"Products                      =  <@ModuleProducts>\n" if ($Debug);

   return @ModuleProducts;
}

########################################################################################
#         sub: getProducts
#        gets: -
#    optional: -
#     returns: @ModuleProducts
# description: returns the products of the whole depending modules
#              each found product name may occurs only once in the module products array
########################################################################################
sub getProducts
{
   my $self = shift;
   my $ProductContent = undef;
   my @tempProducts = ();
   my @ModuleProducts = ();
   my %Products = ();

   my $ModuleData_ref = $$self{"ModuleData"};

   # check whether that the <module-depend> tag exists
   if ($self->existsTag("module-depend"))
   {
      my ($IndexValue, $ModuleDepend_ref) = $self->getIterationData("module-depend");

      for (my $i = 0; $i < $IndexValue; $i++)
      {
         my $Content_ref = getContent($ModuleDepend_ref, $i);

         $ProductContent = getAttribute($Content_ref, "product");

         # if the attribute 'product' wasn't set in the internal data structure,
         # it means the default of 'all' is valid and includes all other possible values
         # but here we need only all 'not-all' values!
         if (!($ProductContent))
         {
            $ProductContent="";
         }
         else
         {
            # here are the products of the current depending module
            @tempProducts = split /\s+/, $ProductContent;

            foreach my $Product (@tempProducts)
            {
               $Products{$Product} = "";
            }
         }
      }

      # fill the sorted 'module products' in the array
      @ModuleProducts = sort keys %Products;
   }

   print"All ModuleProducts            =  <@ModuleProducts>\n" if ($Debug);

   return @ModuleProducts;
}

########################################################################################
#         sub: getJobDirectories
#        gets: $JobType, $JobPlatform
#    optional: $JobType, $JobPlatform(default: 'all' -> includes all possible values)
#     returns: @JobDirectories
# description: creates and returns an array with the sorted directories, which
#              fulfil the expected values of the job type and the job platform
########################################################################################
sub getJobDirectories
{
   my $self = shift;
   my $JobType = shift;
   my $JobPlatform = shift || "all";

   my @JobDirectories = ();

   my $ModuleData_ref = $$self{"ModuleData"};

   # check whether that the <build> tag exists
   if ($self->existsTag("build"))
   {
      # change all possible upper cases to lower cases
      $JobType =~ s/($JobType)/\L$JobType/ if ($JobType);
      $JobPlatform =~ s/($JobPlatform)/\L$JobPlatform/ if ($JobPlatform ne "all");

      my $PlatformContent = undef;
      my %tempJobDirectories = ();

      # get the ref of the <build> tag
      my ($IndexValue, $Build_ref)  = $self->getIterationData("build");

      for (my $i = 0; $i < $IndexValue; $i++)
      {
         my $Content_ref = getContent($Build_ref, $i);

         my $PlatformContent = getAttribute($Content_ref, "platform");
         my $existingDir = getAttribute($Content_ref, "dir");

         # three cases are possible...
         if ($JobType)
         {
            my $JobType_ref = getContent($Content_ref, 0);
            my $existingJobType = getTagName($JobType_ref);

            # if the attribute 'job platform' wasn't set in the internal data structure,
            # it means the default of 'all' is valid and includes all other possible values
            $PlatformContent = "all" if (!($PlatformContent));

            # first case: we get from the caller the parameters 'job type' and 'job platform'
            if ($JobPlatform ne "all")
            {
               # if the caller wants all e.g.'wnt' job platform based directories,
               # we have to get the 'wnt' or the 'all' matching platform
               # because 'all' includes also 'wnt'
               if ( ($JobType eq $existingJobType) &&
                    (($PlatformContent =~ /\b($JobPlatform)\b/) || ($PlatformContent eq "all")) )
               {
                  $tempJobDirectories{$existingDir} = "";
               }
            }
            # second case: we get from the caller only the 'job type' parameter
            #              'job platform' is now 'all'(default) and includes all possible values
            elsif ($JobType eq $existingJobType)
            {
               $tempJobDirectories{$existingDir} = "";
            }
         }
         # third case: we get from the caller no parameter; now we take each existing 'job directory',
         #             no matter which 'job type' and 'job platform' it has
         else
         {
            $tempJobDirectories{$existingDir} = "";
         }
      }

      # sort each unique 'job directory' alphabetical
      @JobDirectories = sort keys %tempJobDirectories;
   }
   print"JobDirectories                =  <@JobDirectories>\n" if ($Debug);

   return @JobDirectories;
}

########################################################################################
#         sub: getDirDependencies
#        gets: $Dir, $JobType, $JobPlatform
#    optional: $JobPlatform(default: 'all' -> includes all possible values)
#     returns: @JobDependencies
# description: creates and returns an array with the sorted depending directories
########################################################################################
sub getDirDependencies
{
   my $self = shift;
   my $Dir = shift;
   my $JobType = shift;
   my $JobPlatform = shift || "all";

   my @JobDependencies = ();

   my $ModuleData_ref = $$self{"ModuleData"};

   # check whether that the <build> tag exists
   if ($self->existsTag("build"))
   {
      # change all possible upper cases to lower cases
      $JobType =~ s/($JobType)/\L$JobType/;
      $JobPlatform =~ s/($JobPlatform)/\L$JobPlatform/ if ($JobPlatform ne "all");

      my $PlatformContent = undef;
      my %tempJobDependencies = ();

      # first we need a reference of the higher level tag <build>
      my ($IndexValue, $Build_ref) = $self->getIterationData("build");

      # get all 'job directories' with the matching values of the 'job type' and the 'job platform'
      my @tempDepDirs = ();
      @tempDepDirs = $self->getJobDirectories($JobType, $JobPlatform);

      # get each content of the <build> tag
      for (my $i = 0; $i < $IndexValue; $i++)
      {
         # get the ref of the content of the <build> tag
         my $Task_ref = getContent($Build_ref, $i);

         # get both attributes: 'job dir' and 'job platform'
         my $existingDir = getAttribute($Task_ref, "dir");
         my $PlatformContent = getAttribute($Task_ref, "platform");

         # if the attribute 'job platform' wasn't set in the internal data structure,
         # it means the default of 'all' is valid and includes all other possible values
         $PlatformContent = "all" if (!($PlatformContent));

         # get the 'job type' ref which is inside the <task> tag on position '0'
         my $JobType_ref = getContent($Task_ref, 0);

         my $existingJobType = getTagName($JobType_ref);

         if ( ($Dir eq $existingDir) && ($JobType eq $existingJobType) )
         {
            # each 'job type' can have several 'depends' and 'build requirements'
            # here we get the number of the including elements
            my $IndexEnd = getContentCount($JobType_ref);

            for (my $j = 0; $j < $IndexEnd; $j++)
            {
               # create a ref of the existing content
               my $Content_ref = getContent($JobType_ref, $j);

               # the content_ref can be 'depend' or 'build requirement'
               # but we only need the 'depend' informations
               next if (getTagName($Content_ref) ne "depend");

               # get the 'depend dir'
               my $DependDir = getAttribute($Content_ref, "dir");

               # look in the list of all existing 'job directories'
               foreach my $DepDir (@tempDepDirs)
               {
                  # get it, if one of these 'job dirs' is equal with one of the 'depending dirs'
                  if ($DepDir eq $DependDir)
                  {
                     # get all unique values only once
                     $tempJobDependencies{$DepDir} = "";
                  }
               }
            }
         }
      }

      # get the unique sorted values in the array
      @JobDependencies = sort keys %tempJobDependencies;
   }

   print"Depending Dirs                =  <@JobDependencies>\n" if ($Debug);

   return @JobDependencies;
}

########################################################################################
#         sub: getJobTypes
#        gets: $Dir
#    optional: -
#     returns: @JobTypes
# description: creates and returns an array with the sorted 'job types'
########################################################################################
sub getJobTypes
{
   my $self = shift;
   my $Dir = shift;

   my @JobTypes = ();

   my $ModuleData_ref = $$self{"ModuleData"};

   # check whether that the <build> tag exists
   if ($self->existsTag("build"))
   {
      # it's for creating unique 'job types' which exists only once in the (later) array
      my %tempJobTypes = ();

      # first we need a reference of the higher level tag <build>
      my ($IndexValue, $Build_ref)  = $self->getIterationData("build");

      for (my $i = 0; $i < $IndexValue; $i++)
      {
         # get the ref of the <build> tag
         my $Task_ref = getContent($Build_ref, $i);
         my $existingDir = getAttribute($Task_ref, "dir");

         # we only need the 'task(s)' with the matching dir
         next if ($Dir ne $existingDir);

         # get the ref of the <task> tag at the position '0'
         my $JobType_ref = getContent($Task_ref, 0);
         my $JobType = getTagName($JobType_ref);

         # get the 'job type' as a key in the hash
         # so we can guarantee that each 'job type' stays unique!
         $tempJobTypes{$JobType} = "";
      }

      # fill the unique sorted 'job types' in the array
      @JobTypes = sort keys %tempJobTypes;
   }

   print"JobTypes                      =  <@JobTypes>\n" if ($Debug);

   return @JobTypes;
}

########################################################################################
#         sub: getJobBuildReqs
#        gets: $Dir, $BuildReqPlatform
#    optional: $BuildReqPlatform(default: 'all' -> includes all possible values)
#     returns: @JobBuildRequirements
# description: creates and returns an array with the sorted 'job build requirements'
########################################################################################
sub getJobBuildReqs
{
   my $self = shift;
   my $Dir = shift;
   my $BuildReqPlatform = shift || "all";

   my @JobBuildRequirements = ();

   my $ModuleData_ref = $$self{"ModuleData"};

   # check whether that the <build> tag exists
   if ($self->existsTag("build"))
   {
      # change all possible upper cases to lower cases
      $BuildReqPlatform =~ s/($BuildReqPlatform)/\L$BuildReqPlatform/ if ($BuildReqPlatform ne "all");

      my $BuildReqPlatformContent = undef;
      my %tempJobBuildRequirements = ();

      # first we need a reference of the higher level tag <build>
      my ($IndexValue, $Build_ref)  = $self->getIterationData("build");

      for (my $i = 0; $i < $IndexValue; $i++)
      {
         # get the ref to the content of the array of the <build> tag
         my $Task_ref = getContent($Build_ref, $i);

         # get the attribute 'task dir'
         my $existingDir = getAttribute($Task_ref, "dir");

         # get the 'job type' ref which is inside the <task> tag
         my $JobType_ref = getContent($Task_ref, 0);

         # each 'job type' can have several 'depends' and 'build requirements'
         # here we get the number of the included elements
         my $IndexEnd = getContentCount($JobType_ref);

         for (my $j = 0; $j < $IndexEnd; $j++)
         {
            # create a ref of the existing content
            my $Content_ref = getContent($JobType_ref, $j);

            # the content_ref can be 'build requirement' or 'depend'
            # but we need only the 'build requirement' informations
            next if (getTagName($Content_ref) ne "build-requirement");

            my $BuildReqName = getAttribute($Content_ref, "name");
            $BuildReqPlatformContent = getAttribute($Content_ref, "platform");

            $BuildReqPlatformContent = "all" if (!($BuildReqPlatformContent));

            if ($BuildReqPlatform ne "all")
            {
               # compare the parameters: 'dir' and 'platform'
               # other values('wnt', 'unx' and 'mac') for 'platform'
               # including the value 'all' in the existing 'platform' list
               # get each 'build requirement name' only once (unique)
               if ( ($Dir eq $existingDir) &&
                    (($BuildReqPlatformContent =~ /\b($BuildReqPlatform)\b/) || ($BuildReqPlatformContent =~ /\ball\b/)) )
               {
                  $tempJobBuildRequirements{$BuildReqName} = "";

                  print"JobBuildRequirements   (if)   =  <$BuildReqName>\n" if ($Debug);
               }
            }
            # if the 'build requirement platform' was not allocated, it is "all" (default)
            # now we only need to compare the directories
            elsif ($Dir eq $existingDir)
            {
               $tempJobBuildRequirements{$BuildReqName} = "";

               print"JobBuildRequirements (elsif)  =  <$BuildReqName>\n" if ($Debug);
            }
         }
      }

      # fill the unique sorted 'build requirement names' in the array
      @JobBuildRequirements = sort keys %tempJobBuildRequirements;
   }

   print"JobBuildRequirements          =  <@JobBuildRequirements>\n" if ($Debug);

   return @JobBuildRequirements;
}

########################################################################################
#         sub: getJobBuildReqPlatforms
#        gets: $Dir, $BuildReqName
#    optional: -
#     returns: @JobBuildReqPlatforms
# description: creates and returns an array with
#              the sorted 'job build requirement platforms'
########################################################################################
sub getJobBuildReqPlatforms
{
   my $self = shift;
   my $Dir = shift;
   my $JobBuildReqName = shift;

   my @JobBuildReqPlatforms = ();

   my $ModuleData_ref = $$self{"ModuleData"};

   # check whether that the <build> tag exists
   if ($self->existsTag("build"))
   {
      # change all possible upper cases to lower cases
      $JobBuildReqName =~ s/($JobBuildReqName)/\L$JobBuildReqName/;

      my $BuildReqPlatformContent = undef;
      my @tempPlatforms = ();
      my %tempJobBuildReqPlatforms = ();

      # first we need a reference of the higher level tag <build>
      my ($IndexValue, $Build_ref)  = $self->getIterationData("build");

      for (my $i = 0; $i < $IndexValue; $i++)
      {
         # get the ref to the content of the array of the <build> tag
         my $Task_ref = getContent($Build_ref, $i);

         # get the attribute 'task dir'
         my $existingDir = getAttribute($Task_ref, "dir");

         # get the 'job type' ref which is inside the <task> tag
         my $JobType_ref = getContent($Task_ref, 0);

         # each 'job type' can have several 'depends' and 'build requirements'
         # here we get the number of the included elements
         my $IndexEnd = getContentCount($JobType_ref);

         for (my $j = 0; $j < $IndexEnd; $j++)
         {
            # create a ref of the existing content
            my $Content_ref = getContent($JobType_ref, $j);

            # the content_ref can be 'build requirement' or 'depend'
            # but we need only the 'build requirement' informations
            next if (getTagName($Content_ref) ne "build-requirement");

            my $existingJobBuildReqName = getAttribute($Content_ref, "name");
            $BuildReqPlatformContent = getAttribute($Content_ref, "platform");

            $BuildReqPlatformContent = "all" if (!($BuildReqPlatformContent));

            if ( ($Dir eq $existingDir) && ($JobBuildReqName eq $existingJobBuildReqName) )
            {
               # here are the platforms of the current 'build requirement'
               @tempPlatforms = split /\s+/, $BuildReqPlatformContent;

               foreach my $BuildReqPlatform (@tempPlatforms)
               {
                  $tempJobBuildReqPlatforms{$BuildReqPlatform} = "";
               }                                                                                                                                        #########
            }
         }
      }

      # fill the unique sorted 'build requirement platforms' in the array
      @JobBuildReqPlatforms = sort keys %tempJobBuildReqPlatforms;
   }

   print"JobBuildReqPlatforms          =  <@JobBuildReqPlatforms>\n" if ($Debug);

   return @JobBuildReqPlatforms;
}

########################################################################################
#         sub: getJobPlatforms
#        gets: $Dir
#    optional: -
#     returns: @JobPlatforms
# description: creates and returns an array with the sorted depending 'job platforms'
########################################################################################
sub getJobPlatforms
{
   my $self = shift;
   my $Dir = shift;

   my @JobPlatforms = ();

   my $ModuleData_ref = $$self{"ModuleData"};

   # check whether that the <build> tag exists
   if ($self->existsTag("build"))
   {
      my $PlatformContent = undef;
      my %tempJobPlatforms = ();

      # control variable: if a value 'all' exists in the platform content
      #                   it doesn't matter which platforms are also existing,
      #                   because 'all' includes all possible values!
      my $allExists = 0;

      # first we need a reference of the higher level tag <build>
      my ($IndexValue, $Build_ref) = $self->getIterationData("build");

      for (my $i = 0; $i < $IndexValue; $i++)
      {
         my $Task_ref = getContent($Build_ref, $i);

         # get the attributes of the <task> tag
         my $existingTaskDir = getAttribute($Task_ref, "dir");
         $PlatformContent = getAttribute($Task_ref, "platform");

         # if it is not set in the data structure,
         # it has automatically the default value 'all'
         $PlatformContent = "all" if (!($PlatformContent));

         if ($Dir eq $existingTaskDir)
         {
            # if a platform value 'all' exists, we remember it
            # and don't look further after other platforms
            if ($PlatformContent =~ /\ball\b/)
            {
               $allExists = 1;

               @JobPlatforms = "all";

               last;
            }

            my @tempPlatforms = ();

            push(@tempPlatforms, split(/\s+/, $PlatformContent));

            foreach my $Platform (@tempPlatforms)
            {
               $tempJobPlatforms{$Platform} = "";
            }
         }
      }

      # fill the unique sorted 'job platforms' in the array,
      # but only if the content "all" is not present in the platform content
      @JobPlatforms = sort keys %tempJobPlatforms if ($allExists == 0);
   }

   print"JobPlatforms                  =  <@JobPlatforms>\n" if ($Debug);

   return @JobPlatforms;
}

########################################################################################
#                                 end of 'get' methods                                 #
########################################################################################

1 ;
