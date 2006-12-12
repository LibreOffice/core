:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: build_list_converter.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2006-12-12 16:31:09 $
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


#*****************************************************************************************
# ASCII parser for the changeover of the current build.lst files to XML files            *
# programmer: Pascal Junck, Sun Microsystems GmbH                                        *
#*****************************************************************************************

# this is the first step for the changeover of the current 'build.lst' files to the new
# 'build.xlist'(XML) files
# before we create the new ones we have to parse all important informations from the old files
# important parameters are:
# 1. 'module name'
# 2. 'module dependency names'
# 3. 'dependency type'
# 4. 'job dir'
# 5. 'depending directories'
# 6. 'job platform'(only: 'w', 'u', 'm' and 'all')
# 7. 'job'(only: 'nmake' means 'make')
# 8. 'build requirements'(here called: 'restrictions')


#################################  begin of main   #######################################

use strict;
use lib ("/home/vg119683/work/modules");

use XMLBuildListParser;

# get and work with each argument(build.lst files) of the commando line
# e.g. if the user wants to parse the build.lst file(s):
#   user input (on unix) for all modules    : 'perl -w ascii_parser.pl /so/ws/SRC680/src.m42/*/prj/build.lst'
#   user input (on windows) for one module  : 'perl -w ascii_parser.pl O:/SRC680/src.m42/[module]/prj/build.lst'
# get all arguments (build.lst files) of the commando line in this global variable '@buildlist_files'
my @buildlist_files = @ARGV;
# global variable for each file name that we want to parse in ASCII
my $parse_file = "";
# set the global variable '$debug' (= 1) to see all results on the terminal,
# else (= 0) it shows nothing of the working output!
my $debug = 0;

# open the filehandle 'ERROR_LOG' for all errors
open (ERROR_LOG, ">>ascii_parse.log")
  or die "Error. Open the file <ascii_parse.log> wasn't successful!\n\n";

# reference of the instance of a new object
my $XMLTree;

foreach (@buildlist_files)
{
   # get each element (= module) in '$parse_file'
   $parse_file = $_;

   # open the filehandle 'PARSE_ASCII' for each module/file that we want to parse
   open (PARSE_ASCII, $parse_file)
     or die "Error. Open the module <$parse_file> wasn't successful!\n\n";

   # create a new object
   $XMLTree = XMLBuildListParser->new();

   # invoking of the main subroutine
   reading_file();

   # is the file name 'build.lst' in the path on the command line?
   # if not, we can not parse and create the new 'build.xlist' file
   if ($parse_file =~ /build(\w+)?\.lst\S*$/)
   {
      my $path = $parse_file;

      $path =~ s/build(\w+)?\.lst\S*$/build\.xlist/;

      $XMLTree->saveXMLFile($path);
   }
   else
   {
      add_errorlog_no_buildlst_file_found_statement($parse_file);
   }

   # close the current $parse_file
   close(PARSE_ASCII);

}
# after all files were read close the errorlog file
close(ERROR_LOG);


###########################  begin of subroutines  #####################################
#
#                       global used variable: $parse_file
#
########################################################################################


########################################################################################
#       sub: reading_file
#      gets: $_ (current file)
#   returns: -
########################################################################################
sub reading_file
{
   # variable for the name of the current module
   my $module_name = "";

   # hashes for the infos beginning at the second line of the build.lst files
   my %dir_of_job_platform = ();
   my %dir_of_alias = ();

   # control variable for the module dependency line of the file
   #   like line 1 of module 'sal' (dependencies means the colon(s)) (SRC680/src.m42)
   #   "sa      sal     :       xml2cmp NULL"
   my $module_dependency_line_exists = 0;
   my $module_dependency_line_was_read = 0;

   # this line variables are for checking that all lines will be read
   # counts each line
   my $line_number = 0;
   # for the sum of the informative lines (='module dependency line' and 'nmake' lines of the file)
   my $info_line_sum = 0;
   # for the sum of the no-info lines, like:
   # job lines: 'usr1', 'get', ... and comment lines: '# ...' or empty lines
   my $no_info_line_sum = 0;

   # read all lines of the file to resolve the first alias
   # with the matching dir to know all aliases and directories
   # at the later second file reading
   while (<PARSE_ASCII>)
   {
      # the variable for each line of a file
      my $line = $_;

      # count each line for more exact error descriptions in the log file
      $line_number += 1;

      # remember it, if this line exists
      if ( (is_module_dependency_line($line)) && ($module_dependency_line_exists == 0) )
      {
         $module_dependency_line_exists = 1;

         # get the name of the current module
         $module_name = get_module_name($line);
      }

      # skip all lines, that hasn't the job 'nmake'
      next if (!(is_nmake_line($line)));

      # check that the infos (job directory, job platform and alias) exist
      if (my ($job_dir, $job_platform, $alias) = get_alias_resolving_infos($line))
      {
         # prove that it's a valid job_platform
         # and that each first alias and matching job platform exists only once
         check_alias_and_job_platform($job_dir, $job_platform, $alias, \%dir_of_job_platform,
                                      \%dir_of_alias, $module_name, $line_number);
      }
      else
      {
         chomp;
         add_errorlog_unknown_format_statement($line, $module_name, $line_number);
         next;
      }
   }
   # reset the $line_number, because we count it again
   $line_number = 0;

   # read the same file again
   seek (PARSE_ASCII,0,0);

   # read each line of the file for all other informations
   # e.g. line 8 of module 'sal'
   # "sa  sal\systools\win32\guistdio  nmake  -     n          sa_guistdio     sa_uwinapi.n          NULL"
   #             $job_dir              $job     $job_platform   1.$alias   2.$alias + alias platform
   while (<PARSE_ASCII>)
   {
      # the variable for each line of a file
      my $line = $_;

      # count each line to check at the end of the file that all lines were read
      # and for more exact error descriptions in the log file
      $line_number += 1;

      # is it a 'nmake' or a 'module dependency' line?
      # if not: print this line to STDOUT,
      #         count one to the no-info lines,
      #         try to get the information about the module name from this line
      #         and skip the line
      if ( (!(is_nmake_line($line))) && (!(is_module_dependency_line($line))) )
      {
         my $no_info_line = show_no_info_line($line, $line_number);

         $no_info_line_sum += $no_info_line;

         # if no module dependency line exists get the name of the current module from another line
         $module_name = get_module_name($line) if (!($module_name));

         # skip the no-info line
         next;
      }

      # only if the module dependency line exists and it wasn't read get the infos about it
      if ( ($module_dependency_line_exists) && (!($module_dependency_line_was_read)) )
      {
         ($module_dependency_line_was_read, $info_line_sum) = get_module_dependency_line_infos
                                                              ($line, $module_name, $line_number);
      }

      # get all 'nmake' line infos
      my $info_line = get_nmake_line_infos($line, \%dir_of_alias, \%dir_of_job_platform,
                                           $module_name, $line_number);

      # count the info lines;
      $info_line_sum += $info_line;
   }

   if ($debug == 1)
   {
      # show the sums of the info and no-info lines
      lines_sums_output($module_name, $line_number, $info_line_sum, $no_info_line_sum);
   }
}

########################################################################################
#       sub: is_module_dependency_line
#      gets: $line
#   returns: 1 (true) or 0 (false)
########################################################################################
sub is_module_dependency_line
{
   my $line = shift;

   # if the module dpendency line exists return 1, otherwise 0
   ($line =~ /^\w+\s+\S+\s+:+\s+/)
   ? return 1
   : return 0;
}

########################################################################################
#       sub: is_nmake_line
#      gets: $line
#   returns: '1' (true) or '0' (false)
########################################################################################
sub is_nmake_line
{
   my $line = shift;

   # these lines are NO nmake lines:
   # 1. a empty line
   # 2. a comment line (perhaps with the job 'nmake')
   #    like line 20 of module 'bridges' (SRC680/src.m42)
   #    "#br  bridges\source\cli_uno  nmake  -  w,vc7  br_cli_uno br_unotypes NULL========= "
   # 3. the module dependency line
   #    like line 1 of module 'sal' (dependencies means the colon(s)) (SRC680/src.m42)
   #    "sa      sal     :       xml2cmp N                                    ULL"
   # 4. a 'p' job platform line (for OS2)
   # 5. a line with a job, which is not 'nmake'
   ($line =~ (/^[^\s+\#]/) && (!(/\s+:+\s+/)) && (!(/\s+p\s+/)) && (/\bnmake\b/) )
   ? return 1
   : return 0;
}

########################################################################################
#       sub: get_alias_resolving_infos
#      gets: $line
#   returns: $job_dir, $job_platform, $alias
########################################################################################
sub get_alias_resolving_infos
{
   my $line = shift;

   if ($line =~ /^\w+\s+(\S+)\s+\w+\s+\-\s+(\w+)\,?(\w+)?\s+(\S+)\s+/)
   {
      # get the current work directory
      my $temp_job_dir = $1;

      my $job_dir = change_job_directory($temp_job_dir);

      # get the job platform of the current job
      # if it is a 'n' job platform transform it to 'w'
      # because 'n' can be used now as 'w' (both means windows)
      my $job_platform = $2;
      $job_platform = "w" if($job_platform eq "n");

      # get the first alias in each line
      my $alias = $4;

      return ($job_dir, $job_platform, $alias);
   }
   return (undef, undef, undef);
}

########################################################################################
#       sub: change_job_directory
#      gets: $job_dir
#   returns: $changed_job_dir
########################################################################################
# we don't need the module name and the first '\' in the current directory
sub change_job_directory
{
   my $changed_job_dir = shift;

   # ignore the module name
   $changed_job_dir =~ s/^\w+//;
   # change all other '\' against the '/' of the current dir
   $changed_job_dir =~ s/\\/\//g;

   # get only a "/" if we are in the root directory
   $changed_job_dir = "/" if ($changed_job_dir eq "");

   return $changed_job_dir;
}

########################################################################################
#       sub: check_alias_and_job_platform
#      gets: $job_dir, $job_platform, $alias, $dir_of_job_platform_ref,
#            $dir_of_alias_ref, $module_name, $line_number
#   returns: -
########################################################################################
# get it in the hash only if it is a valid job platform,
# like 'w', 'u', 'm' and 'n'
# 'all' is also valid but it doesn't exist in an alias platform(!)
sub check_alias_and_job_platform
{
   my ($job_dir, $job_platform, $alias, $dir_of_job_platform_ref,
       $dir_of_alias_ref, $module_name, $line_number) = @_;

   # is it a valid job_platform?
   if ($job_platform =~ /(w|u|m|n|all)/)
   {
      # get only the 'w', 'u', 'm' and 'n' based job platforms
      if ($job_platform =~ /[wumn]/)
      {
         # doesn't the key already exist?
         (!(exists $$dir_of_job_platform_ref{$job_platform.$alias}))
           # get the first alias with the matching job platform in the hash
         ? get_alias_and_job_platform($job_platform, $alias, $dir_of_job_platform_ref)
           # this is a line with a redundant alias and job platform
         : add_errorlog_alias_redundancy_statement($module_name, $alias, $job_platform, $line_number);
      }
      if (!(exists $$dir_of_alias_ref{$alias}))
      {
         # get each first alias with the matching job platform
         get_alias_and_matching_directory($dir_of_alias_ref, $alias, $job_dir);
      }
   }
   # it's not a valid job platform
   else
   {
      add_errorlog_invalid_platform_statement($module_name, $job_platform, $line_number);
   }
}

########################################################################################
#       sub: get_alias_and_job_platform
#      gets: $job_platform, $alias, $dir_of_job_platform_ref
#   returns: -
########################################################################################
# get the the job platform and the first alias as a unique key
# and the job platform as value of the hash
# it's for checking later that the alias platform is equal to the job platform
#   e.g.: line 6 + 7 of the module 'gtk' (SRC680/src.m42)
#   "gt  gtk\pkgconfig  nmake  -  u   gt_pkg   NULL"
#   "gt  gtk\glib       nmake  -  u   gt_glib gt_pkg.u NULL"
#   the alias 'gt_pkg' has the directory 'gtk\pkgconfig' (we need only 'pkgconfig')
#   and it has the job platform 'u' - compare it with the alias platform 'gt_pkg.u'
sub get_alias_and_job_platform
{
   my ($job_platform, $alias, $dir_of_job_platform_ref) = @_;

   # key = 'job platform' and 'first alias'   =>   value = 'job platform'
   $$dir_of_job_platform_ref{$job_platform.$alias} = $job_platform;
}

########################################################################################
#       sub: get_alias_and_matching_directory
#      gets: $dir_of_alias_ref, $alias, $job_dir
#   returns: -
########################################################################################
# fill the hash with the first alias and the matching directory
#   e.g. line 14 of module 'setup2' (SRC680/src.m42)
#   "se  setup2\win\source\unloader   nmake   -   w   se_wulo se_unotypes NULL"
#   key = 'se_wulo'     =>    value = 'win/source/unloader'
sub get_alias_and_matching_directory
{
   my ($dir_of_alias_ref, $alias, $job_dir) = @_;

   #     key = 'first alias'  => value = 'job directory'
   $$dir_of_alias_ref{$alias} = $job_dir;
}

########################################################################################
#       sub: show_no_info_line
#      gets: $line, $line_number
#   returns: $no_info_line
########################################################################################
sub show_no_info_line
{
   my ($line, $line_number) = @_;
   my $no_info_line += 1;

   chomp($line);

   print"Ignore line <$line_number>:\n\"$line\"\n\n" if ($debug);

   return $no_info_line;
}

########################################################################################
#       sub: get_module_name
#      gets: $line
#   returns: $module_name
########################################################################################
sub get_module_name
{
   my $line = shift;
   my $module_name = "";

   if ($line =~ /^\w+\s+([\w\.\-]+)\\?/)
   {
      $module_name = $1;
   }

   # set the 'module name' in the data structure tree
   $XMLTree->setModuleName($module_name);

   return $module_name;
}

########################################################################################
#       sub: get_module_dependency_line_infos
#      gets: $line, $module_name, $line_number
#   returns: $module_dependency_line_was_read, $info_line_sum
########################################################################################
# get the informations about the module dependency line
# like line 1 of module 'sal' (SRC680/src.m42)
#    "sa     sal              :                       xml2cmp             NULL"
#        $module_name   $module_dependency    @module_dependency_names
sub get_module_dependency_line_infos
{
   my ($line, $module_name, $line_number) = @_;
   my $module_dependency = "";
   my @module_dependency_names = ();
   my %dep_modules_and_products = ();
   my $product = "";

   my $module_dependency_line_was_read = 1;
   my $info_line_sum = 1;

   if ($debug)
   {
      print"\nline number               : <$line_number>\n";
      print"module-name               : <$module_name>\n";
   }

   # get the dependencies
   if ($line =~ /\s+(:+)\s+/)
   {
      $module_dependency = $1;
      print"module-dependency         : <$module_dependency>\n" if ($debug);

      # transform the dependency type to the corresponding tag name
      if ($module_dependency eq ":")
      {
         $module_dependency = "md-simple";
      }
      elsif ($module_dependency eq "::")
      {
         $module_dependency = "md-always";
      }
      elsif ($module_dependency eq ":::")
      {
         $module_dependency = "md-force";
      }
   }

   # get a list of all depending module names
   if ($line =~ /:+\s+([\S\s]+)\s+NULL/)
   {
      @module_dependency_names = split(/\s+/, $1);

      foreach my $module (@module_dependency_names)
      {
         # check whether that there is another product (as "all") of a module
         if ($module =~ /(\S+):+(\S+)/)
         {
            $dep_modules_and_products{$2} = $1;
         }
         else
         {
            $dep_modules_and_products{$module} = "all";
         }
      }
   }

   # add the dependency module names, the module dependency type and the product to the data structure
   foreach my $module (sort keys %dep_modules_and_products)
   {
      print"module-dependency-name(s) : key <$module>  value <".$dep_modules_and_products{$module}.">\n" if ($debug);

      $XMLTree->addModuleDependencies($module, $module_dependency, $dep_modules_and_products{$module});
   }

   return ($module_dependency_line_was_read, $info_line_sum);
}

########################################################################################
#       sub: get_nmake_line_infos
#      gets: $line, \%dir_of_alias, \%dir_of_job_platform, $module_name, $line_number
#   returns: $info_line
########################################################################################
# get all infos about the 'nmake' lines
# e.g. line 8 of module 'sal'
# "sa  sal\systools\win32\guistdio  nmake  -     n         sa_guistdio     sa_uwinapi.n          NULL"
#             $job_dir              $job     $job_platform  1.$alias   2.$alias + alias platform
sub get_nmake_line_infos
{
   my ($line, $dir_of_alias_ref, $dir_of_job_platform_ref, $module_name, $line_number) = @_;
   my $directories_ref = "";
   my $info_line = 0;

   # get the infos about the 'nmake' lines
   if ($line =~ /^\w+\s+(\S+)\s+(\w+)\s+\-\s+(\w+)\,?(\S+)?/)
   {
      # get the current working directory
      my $temp_job_dir = $1;
      my $job_dir = change_job_directory($temp_job_dir);

      # get the job
      my $job = $2;
      $job = "make" if ($job eq "nmake");

      # get the job platform of the current job
      # if it is a 'n' job platform transform it to 'wnt'
      # available values are: 'wnt', 'unx', 'mac' or 'all'
      my $job_platform = $3;
      $job_platform = change_job_platform_name($job_platform);

      # get the first alias in each line
      my $restriction = $4;
      my %build_req = ( "$restriction" => "$job_platform") if ($restriction && $job_platform);


      # get all aliases (but not the first) in an array
      my $aliases_ref = get_aliases($line);

      # filter the list of aliases, which has a 'p' job platform
      # and transform a 'n' ending alias platform to a 'w' ending alias platform
      filter_aliases($aliases_ref);

      # resolve all aliases (alias[.job platform] => matching directory)
      $directories_ref = resolve_aliases($aliases_ref, $dir_of_alias_ref,
                                         $dir_of_job_platform_ref, $module_name, $line_number);

      # count the informative lines
      $info_line = 1;

      $XMLTree->addJob($job_dir, $job, $job_platform, $directories_ref, \%build_req);

      # show the infos, that we know about each line
      if ($debug == 1)
      {
         show_line_infos($line_number, $job_dir, $job, $job_platform, $restriction, $aliases_ref, $directories_ref);
      }
   }
   return $info_line;
}

########################################################################################
#       sub: change_job_platform_name
#      gets: $job_platform
#   returns: $job_platform
########################################################################################
sub change_job_platform_name
{
   my $job_platform = shift;

   $job_platform = "wnt" if($job_platform eq "n" || $job_platform eq "w");
   $job_platform = "unx" if($job_platform eq "u");
   $job_platform = "mac" if($job_platform eq "m");

   return $job_platform;
}

########################################################################################
#       sub: get_aliases
#      gets: $_ (current line)
#   returns: \@aliases
########################################################################################
# get all aliases of the line in an array
sub get_aliases
{
   my $line = shift;
   my @aliases = ();

   # get all aliases in an array (but cut out the first alias)
   if ($line =~ /\-\s+[\w+\,]+\s+([\S\s]+)\s+NULL$/)
   {
      print"\nall job aliases           : <$1>\n" if ($debug);

      @aliases = split /\s+/, $1;

      # we don't need the first alias (it stands for the current job directory)
      shift @aliases;
   }
   return \@aliases;
}

########################################################################################
#       sub: filter_aliases
#      gets: $aliases_ref
#   returns: -
########################################################################################
# filter all aliases, because we only need the 'w', 'u' and 'm' job platform based aliases
sub filter_aliases
{
   my $aliases_ref = shift;

   # get the highest index of the array (number of elements of the array - 1)
   # also works: my $index = scalar(@$aliases_ref)-1;
   my $index = $#{@{$aliases_ref}};

   for (; $index >= 0; $index--)
   {
      # filter the 'p' job platform based aliases from '@aliases'
      splice(@$aliases_ref, $index, 1) if ($$aliases_ref[$index] =~ /\.p$/);

      # transform a '.n' ending alias platform to '.w' ending alias platform
      if ($$aliases_ref[$index] =~ /\.n$/)
      {
         $$aliases_ref[$index] =~ s/\.n$/\.w/;
         splice(@$aliases_ref, $index, 1, $$aliases_ref[$index]);
      }
   }
}

########################################################################################
#       sub: resolve_aliases
#      gets: $aliases_ref, $dir_of_alias_ref, $dir_of_job_platform_ref,
#            $module_name, $line_number
#   returns: \@directories
########################################################################################
# here we get each alias with the matching job directory
sub resolve_aliases
{
   my ($aliases_ref, $dir_of_alias_ref, $dir_of_job_platform_ref, $module_name, $line_number) = @_;

   my @directories = ();
   my ($alias_platform, $alias, $temp_alias) = "";

   # resolving all directory aliases
   foreach $temp_alias (@$aliases_ref)
   {
      ($alias, $alias_platform) = compare_job_platform_with_alias_platform
                                  ($temp_alias, $dir_of_job_platform_ref, $module_name, $line_number);

      # does the alias exist?
      if (exists $$dir_of_alias_ref{$alias})
      {
         # then get the matching directory in the array
         push (@directories, $$dir_of_alias_ref{$alias});
      }
      else
      {
         add_errorlog_no_directory_of_alias_statement($module_name, $alias, $line_number);
      }
   }
   return \@directories;
}

########################################################################################
#       sub: compare_job_platform_with_alias_platform
#      gets: $alias, $dir_of_job_platform_ref, $module_name, $line_number
#   returns: $alias
########################################################################################
sub compare_job_platform_with_alias_platform
{
   my ($alias, $dir_of_job_platform_ref, $module_name, $line_number) = @_;

   my $alias_platform = "";

   # compare the alias platform (with a dot and an ending letter, like "al_alib.u")
   # with the job platform of the line in which this alias was resolved
   if ($alias =~ /\.([wum])$/)
   {
      $alias_platform = $1;

      # don't memorize the ending dot and letter
      $alias =~ s/\.\w$//;

      # if the value(= job platform) of the hash or the alias platform has no value
      # set it to "no valid value"
      if (!(exists $$dir_of_job_platform_ref{$alias_platform.$alias}))
      {
         $$dir_of_job_platform_ref{$alias_platform.$alias} = "no valid value";
      }
      $alias_platform = "no valid value" if (!($alias_platform));

      # are the job platform and the alias platform equal?
      if ($$dir_of_job_platform_ref{$alias_platform.$alias} ne $alias_platform)
      {
         add_errorlog_not_equal_platforms_statement
         ($module_name, $alias, $alias_platform, $dir_of_job_platform_ref, $line_number);
      }
   }
   return ($alias, $alias_platform);
}

########################################################################################
#       sub: show_line_infos
#      gets: $line_number, $job_dir, $job, $job_platform, $restriction,
#            $aliases_ref, $directories_ref
#   returns: -
########################################################################################
# print the infos about each line
sub show_line_infos
{
   my ($line_number, $job_dir, $job, $job_platform, $restriction, $aliases_ref, $directories_ref) = @_;

   print"line number               : <$line_number>\n";
   print"job directory             : <$job_dir>\n";
   print"job                       : <$job>\n";
   print"job platform              : <$job_platform>\n" if ($job_platform =~ /(w|u|m|all)/);
   print"restriction               : <$restriction>\n" if ($restriction);
   print"alias dependencies        : <@$aliases_ref>\n";
   print"directory dependencies    : <@$directories_ref>\n\n";
}

########################################################################################
#       sub: lines_sums_output
#      gets: $module_name, $line_number, $info_line_sum, $no_info_line_sum
#   returns: -
########################################################################################
sub lines_sums_output
{
   # this line variables are for checking that all lines will be read:
   my ($module_name, $line_number, $info_line_sum, $no_info_line_sum) = @_;
   my $lines_sum = 0;

   add_errorlog_no_module_name_statement() if (!($module_name));

   # were all lines read? and is the checksum okay?
   $lines_sum = $info_line_sum + $no_info_line_sum;
   if ($lines_sum == $line_number)
   {
      print"All $line_number line(s) of module <$module_name> were read and checked!\n\n";
   }
   else
   {
      add_errorlog_different_lines_sums_statement($module_name);
   }

   print"module: <$module_name>\n".
        "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n".
        "   info line(s) sum    =  $info_line_sum\n".
        "no-info line(s) sum    =  $no_info_line_sum\n".
        "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n".
        "  total line(s) sum    =  $line_number\n\n\n";
}


############################ ERROR_LOG subroutines #####################################


########################################################################################
#       sub: add_errorlog_invalid_job_platform_statement
#      gets: $module_name, $platform, $line_number
#   returns: -
########################################################################################
sub add_errorlog_invalid_job_platform_statement
{
   my ($module_name, $job_platform, $line_number) = @_;

   print ERROR_LOG "Error in module <$module_name> of dir/file <$parse_file> line <$line_number>.\n".
                   "The job platform <$job_platform> is not valid.\n\n";
}

########################################################################################
#       sub: add_errorlog_not_equal_platforms_statement
#      gets: $module_name, $alias, $alias_platform, $dir_of_job_platform_ref, $line_number
#   returns: -
########################################################################################
sub add_errorlog_not_equal_platforms_statement
{
   my ($module_name, $alias, $alias_platform, $dir_of_job_platform_ref, $line_number) = @_;

   print ERROR_LOG "Error in module <$module_name> of dir/file <$parse_file> line <$line_number>.\n".
                   "The alias platform <$alias.$alias_platform> is not equal ".
                   "with the job platform <$$dir_of_job_platform_ref{$alias_platform.$alias}>.\n\n";
}

########################################################################################
#       sub: add_errorlog_no_directory_of_alias_statement
#      gets: $module_name, $alias, $line_number
#   returns: -
########################################################################################
sub add_errorlog_no_directory_of_alias_statement
{
   my ($module_name, $alias, $line_number) = @_;

   print ERROR_LOG "Error in module <$module_name> of dir/file <$parse_file> line <$line_number>.\n".
                   "The directory of the alias <$alias> doesn't exist!\n\n";
}

########################################################################################
#       sub: add_errorlog_no_module_name_statement
#      gets: -
#   returns: -
########################################################################################
sub add_errorlog_no_module_name_statement
{
   print ERROR_LOG "Error. No module name found in dir/file <$parse_file>.\n\n";
}

########################################################################################
#       sub: add_errorlog_alias_redundancy_statement
#      gets: $module_name, $alias, $job_platform, $line_number
#   returns: -
########################################################################################
sub add_errorlog_alias_redundancy_statement
{
   my ($module_name, $alias, $job_platform, $line_number)= @_;

   print ERROR_LOG "Error in module <$module_name> of dir/file <$parse_file> line <$line_number>.\n".
                   "The alias <$alias> with the job platform <$job_platform> is redundant.\n\n";
}

########################################################################################
#       sub: add_errorlog_unknown_format_statement
#      gets: $module_name, $line_number
#   returns: -
########################################################################################
sub add_errorlog_unknown_format_statement
{
   my ($line, $module_name, $line_number) = @_;

   print ERROR_LOG "Error in module <$module_name> of dir/file <$parse_file> line <$line_number>.".
                   "\nUnknown format:\n\"$line\"\n\n";
}

########################################################################################
#       sub: add_errorlog_different_lines_sums_statement
#      gets: $module_name
#   returns: -
########################################################################################
sub add_errorlog_different_lines_sums_statement
{
   my $module_name = shift;

   print ERROR_LOG "Error in module <$module_name> of dir/file <$parse_file>.\n".
                   "The sums of all info and no-info lines are not correct!\n\n";
}

########################################################################################
#       sub: add_errorlog_no_buildlst_file_found_statement
#      gets: $parse_file
#   returns: -
########################################################################################
sub add_errorlog_no_buildlst_file_found_statement
{
   my $parse_file = shift;

   print ERROR_LOG "Error in command line argument <$parse_file>.\n".
                   "File 'build.lst' not found!\n";
}

############################# end of the subroutines ###################################
