#!/usr/bin/perl
#########################################################################

 #*************************************************************************
 #
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
 #*************************************************************************

####################################################################
# File Name: test_driver.pl
# Version  : 1.0
# Project  : Xmerge
# Author   : Brian Cameron
# Date	   : 5th Sept. 2001
#
# This script does the following:
#
# Processes the input file, and runs the tests specified in that
# file.  This will do the following for each test:
#
# 1. Convert a file from XML to PDB format
# 2. Starts up the Palm OS emulator with the appropriate program
#    running and the converted file loaded the program.
# 3. Makes automated changes as specified in the inputfile to
#    this script..
# 4. Returns to the main applications window.
#
# Parameter
#   Filename to convert and change
#
##########################################################################

# Turn on auto-flushing
#
$|=1;

use EmRPC;

# Directory where converterlib is located...
#
use lib "$ENV{qa-dir}/lib";
use converterlib;

#-------------------- Start of main script ------------------------------------

# Environmental Settings

$pose_exe = "$ENV{pose-dir}/pose/posedist/pose";
$pose_prc = "$ENV{thirdpartyapps-dir}";
$test_list = "";
$infile = "";
$merge_opt = 0;

# You may need to change this from the default if your pose emulator
# starts faster or slower than mine.
#
if ($ENV{'POSE_TIMEOUT'})
{
  $pose_timeout  = "$ENV{'POSE_TIMEOUT'}";
}
else
{
  $pose_timeout = 15;
}

$cmdline_len = @ARGV;
if ($cmdline_len <= 0)
{
    print_usage();
    exit (0);
}

&process_cmdline(@ARGV);
&print_env();
&verify_env_options();

# Make the output directories with timestamps included in the
# directory names.
#
mkdir $pdb_orig, 0777 || die "can not create directory <$pdb_new>.";
`chmod 777 $pdb_orig`;
mkdir $pdb_new,  0777 || die "can not create directory <$pdb_new>.";
`chmod 777 $pdb_new`;
mkdir $xml_new,  0777 || die "can not create directory <$pdb_new>.";
`chmod 777 $xml_new`;

&verify_prcs_exist("DBExporter.prc");

if ($test_list ne "")
{
    open (TESTLIST, $test_list) || die "Couldn't open testcase list file $test_list";

    while (<TESTLIST>)
    {
        &process_testcase($_);
    }
}
elsif ($infile ne "")
{
    &process_testcase($infile);
}
else
{
    die ("You didn't supply any test cases to process");
}

print "Finished.\n";
exit(0);

#-------------------- End of main script ----------------------------------------

#--------------------------------------------------------------------------------
# Various sub routines
#--------------------------------------------------------------------------------

# process_testcase
# infile - test case file name
#
# This is the main driver function
# Opens the infile, reads it in parses it, runs the appropriate conversion
# starts pose and load the file into the emulator. It launches the 
# appropriate editor and then runs the commands specified in the test case.
# It then exports the file and saves it locally. Finally it is converted
# back to the original office format.
#
sub process_testcase
{
    my $infile = $_[0];
    my $convert_file = "";
    my $rc;

    # Process the inputfile
    #
    open (INFILE, $infile) || die "Failed to open test case <$infile>";
    
    $running_testtype = "";
    
    # Process the input file.
    #
    while ($c_inline = <INFILE>)
    {
       chomp $c_inline;
       @entry = split('\|', $c_inline);
    
       # Process TEST 
       #
       if ($c_inline =~ /^ *#/ || $c_inline =~ /^[ \t]*$/)
       {
          # skip comments and blank lines.
          #
          next;
       }
       elsif ("$entry[0]" eq "TEST")
       {
          # Close the test if one is running.
          #
          &close_program($convert_file);
          $running_testtype = "";
          
          $valid_test = 0;
    
          if ($#entry != 3)
          {
             print "\nERROR, $entry[0] invalid number of arguments\n\n";
          }
          else
          {
             # Start the test.
             #
             print "\nStarting test: $entry[1]\n";
             $convert_file = $entry[3];
       
             if ("$entry[2]"  =~ /[Qq][Uu][Ii][Cc][Kk][Ww][Oo][Rr][Dd]/)
             {
                $xml_extension = "sxw";
                $convert_to = "doc";
    
                # Convert XML file to pdb format.
                #
                $rc = &convert_to_pdb("$xml_orig", $convert_file, $xml_extension ,
                   $convert_to,"$pdb_orig");
                if ($rc != 0)
                {
                   print "\nERROR, problem converting file $convert_file\n\n";
                }
                else
                {
                   # Start pose
                   #
                   $rc = &start_pose("$pose_exe",
                     "$pose_prc/Quickword.PRC,$pose_prc/DBExporter.prc,$pdb_orig/$convert_file.pdb",
                     "Quickword", $pose_timeout);
    
                   if ($rc == 0)
                   {
                      &start_quickword();
                      $valid_test = 1;
                      $running_testtype = "QUICKWORD";
                      print "\npose launched, begin automated test sequence for QuickWord\n";
                   }
                   else
                   {
                      &kill_pose();
                      $running_testtype = "";
                   }
                }
             }
             elsif ("$entry[2]"  =~ /[Mm][Ii][Nn][Ii][Cc][Aa][Ll][Cc]/)
             {
                $xml_extension = "sxc";
                $convert_to = "minicalc";
    
                # Convert XML file to pdb format.
                #
                $rc = &convert_to_pdb("$xml_orig", $convert_file,
                  $xml_extension, $convert_to,"$pdb_orig");
                if ($rc != 0)
                {
                   print "\nERROR, problem converting file $convert_file\n\n";
                }
                else
                {
                   # Get minicalc PDB file names, since an SXC file can
                   # be converted to more than one.
                   #
                   $pdb_files="";
                   $i = 1;
                   while (-f "$pdb_orig/$convert_file-Sheet$i.pdb")
                   {
                     if ($i > 1)
                     {
                       $pdb_files .= ",";
                     }
                     $pdb_files .= "$pdb_orig/$convert_file-Sheet$i.pdb";
                     $i++;
                   }
                   $number = $i-1;

                   # Start pose
                   #
                   $rc = &start_pose("$pose_exe",
                      "$pose_prc/MiniCalc.prc,$pose_prc/DBExporter.prc,$pdb_files",
                      "MiniCalc", $pose_timeout);
    
                   if ($rc == 0)
                   {
                      &start_minicalc();
                      $valid_test = 1;
                      $running_testtype = "MINICALC";
                      print "pose launched, begin automated test sequence for MiniCalc\n";
                   }
                   else
                   {
                      &kill_pose();
                      $running_testtype = "";
                   }
                }
             }
             else
             {
                print "\nERROR, invalid extension <$entry[2]>\n\n";
             }
          }
       }
    
       # Process DB_EXPORT
       #
       elsif ("$entry[0]" eq "DB_EXPORT")
       {
          if ($#entry != 1)
          {
             print "\nERROR, $entry[0] invalid number of arguments\n\n";
          }
          else
          {
             &db_export($entry[1]);
          }
       }

       # Process TAP_APPLICATIONS
       #
       elsif ("$entry[0]" eq "TAP_APPLICATIONS")
       {
          if ($#entry != 0)
          {
             print "\nERROR, $entry[0] invalid number of arguments\n\n";
          }
          else
          {
             &tap_applications(0);
          }
       }

       # Process ENTER_STRING_AT_LOCATION
       #
       elsif ("$entry[0]" eq "ENTER_STRING_AT_LOCATION")
       {
          if ($#entry != 3)
          {
             print "\nERROR, $entry[0] invalid number of arguments\n\n";
          }
          elsif ($valid_test == 0)
          {
             print "\nERROR, can not process $entry[0] for invalid test\n\n";
          }
          else
          {
             &enter_string_at_location($entry[1], $entry[2],
               $entry[3], $running_testtype);
          }
       }
    
       # Process TAP_PEN
       #
       elsif ("$entry[0]" eq "TAP_PEN")
       {
          if ($#entry != 2)
          {
             print "\nERROR, $entry[0] invalid number of arguments\n\n";
          }
          elsif ($valid_test == 0)
          {
             print "\nERROR, can not process $entry[0] for invalid test\n\n";
          }
          else
          {
             &pose_tap_pen($entry[1], $entry[2], 0);
          }
       }

       # Process TAP_BUTTON
       #
       elsif ("$entry[0]" eq "TAP_BUTTON")
       {
          if ($#entry != 1)
          {
             print "\nERROR, $entry[0] invalid number of arguments\n\n";
          }
          elsif ($valid_test == 0)
          {
             print "\nERROR, can not process $entry[0] for invalid test\n\n";
          }
          else
          {
             &pose_tap_button($entry[1], 0);
          }
       }

       # Process SLEEP
       #
       elsif ("$entry[0]" eq "SLEEP")
       {
          if ($#entry != 1)
          {
             print "\nERROR, $entry[0] invalid number of arguments\n\n";
          }
          else
          {
             &pose_sleep($entry[1]);
          }
       }

       # Process MINICALC_ENTER_CELL
       #
       elsif ("$entry[0]" eq "MINICALC_ENTER_CELL")
       {
          if ($#entry != 3)
          {
             print "\nERROR, $entry[0] invalid number of arguments\n\n";
          }
          elsif ($valid_test == 0)
          {
             print "\nERROR, can not process $entry[0] for invalid test\n\n";
          }
          else
          {
             &minicalc_enter_cell($entry[1], $entry[2], $entry[3]);
          }
       }

       # Process QUICKWORD_FIND_REPLACE
       #
       elsif ("$entry[0]" eq "QUICKWORD_FIND_REPLACE")
       {
          if ($#entry != 2)
          {
             print "\nERROR, $entry[0] invalid number of arguments\n\n";
          }
          elsif ($valid_test == 0)
          {
             print "\nERROR, can not process $entry[0] for invalid test\n\n";
          }
          else
          {
             &quickword_find_replace($entry[1], $entry[2]);
          }
       }
       else
       {
          print "\nERROR, invalid line <$c_inline>\n";
       }
    }
    
    &close_program($convert_file);
}

# close_program
# convert_file - file to export
# 
# closes the program running in pose and kills pose
#
sub close_program
{
    my $convert_file = $_[0];

    if ($running_testtype eq "QUICKWORD")
    {
        print "QuickWord test completed.\n";
        &close_program_quickword($convert_file);
    }
    elsif ($running_testtype eq "MINICALC")
    {
        print "MiniCalc test completed.\n";
        &close_program_minicalc($convert_file, $number);
    }
}

# close_program_quickword
# convert_file - file to export 
# 
# Closes quickword and kills pose
#
sub close_program_quickword
{
      my $convert_file = $_[0];
      my $error_file = "./error.txt";
      my $rc;

      &close_quickword();

      &db_export($convert_file);
      print "Moving /tmp/$convert_file.pdb to $pdb_new\n";
      `mv /tmp/$convert_file.pdb $pdb_new`;
      `chmod 666 $pdb_new/$convert_file.pdb`;

      &close_connection(1);
      &kill_pose();
      print "\nFinishing test...\n";

      # The path of where to put the error file should be specified
      # in the properties file.  Not sure if it is really necessary
      # to put this out to a separate file.  STDOUT should be fine.
      #
      $rc = &convert_to_xml($xml_new, $xml_orig,
        "$pdb_new/$convert_file.pdb", "doc" ,
          "sxw", $convert_file, $merge_opt);
      if ($rc != 0)
      {
        print "\nERROR, problem converting file $pdb_new/$convert_file.pdb\n\n";
      }
}

# close_program_minicalc
# convert_file - file to export
# 
# Closes minicalc and kills pose
#
sub close_program_minicalc
{
      my $convert_file = $_[0];
      my $num_files   = $_[1];
      my $list="";
      my $rc;

      &close_minicalc();

      for ($a=1; $a <= $num_files; $a++)
      {
        &db_export("$convert_file-Sheet$a");
        print "Moving /tmp/$convert_file-Sheet$a.pdb to $pdb_new/\n";
        `mv /tmp/$convert_file-Sheet$a.pdb $pdb_new/`;
        `chmod 666 $pdb_new/$convert_file-Sheet$a.pdb`;
      }

      &close_connection(1);
      &kill_pose();
      print "\nFinishing test...\n";

      for ($a=1; $a <= $num_files; $a++)
      {
        $list .="$pdb_new/$convert_file-Sheet$a.pdb "
      }

      $rc = &convert_to_xml($xml_new, $xml_orig, "$list",
          "minicalc", "sxc", $convert_file, $merge_opt);  
      if ($rc != 0)
      {
        print "\nERROR, problem converting file(s) $list\n\n";
      }

      &pose_sleep(5);
}

# print_usage
#
# prints the usage for this program.
#
sub print_usage
{
    print "Usage : test_driver.pl\n";
    print "\t-test=<file> \t\t: individual test case file to run\n";
    print "\t-list=<file> \t\t: list of test case files\n"; 
    print "\t-env=<file> \t\t: Properites like file defining env\n"; 
    print "\t-pose-exe=<fullpath> \t: path to pose executable\n";
    print "\t-pose-prc=<path> \t: path to directory holding prc files\n";
    print "\t-pdb-orig=<path> \t: directory to hold original pdb files\n";
    print "\t-pdb-new=<path> \t: directory to hold new pdb files\n";
    print "\t-xml-orig=<path> \t: directory to hold original office documents\n";
    print "\t-xml-new=<path> \t: directory to hold new office documents\n";
    print "\t-merge          \t: Invokes the merge option when converting\n";
    print "\t                \t  from PDB back to XML.\n";
}

# print_env
#
# Prints the current environment.
#
sub print_env
{
    print "\nUsing the following environment:\n";
    print "\tPOSE_EXE  = $pose_exe\n";
    print "\tPOSE_PRC  = $pose_prc\n";
    print "\tPDB_ORIG  = $pdb_orig\n";
    print "\tPDB_NEW   = $pdb_new\n";
    print "\tXML_ORIG  = $xml_orig\n";
    print "\tXML_NEW   = $xml_new\n";
}

# process_cmdline
#
# command line options come in as key/value pairs.
# read them and set the appropriate global variable
#
# Sets these globals: pose_exe, pose_prc, xml_orig, xml_new_dir,
# xml_new, pdb_orig_dir, pdb_orig, pdb_new_dir, pdb_new.
#
sub process_cmdline
{
    my $lu_str = &get_date_string();

    foreach $i (@_)
    {
        my @arg= split('=', $i);
        @arg[0] =~ tr/A-Z/a-z/;
        
        if (@arg[0] eq "-pose-exe")
        {
            $pose_exe=$arg[1];
        }
        elsif (@arg[0] eq "-pose-prc")
        {
            $pose_prc=$arg[1];
        }
        elsif (@arg[0] eq "-pdb-orig")
        {
            $pdb_orig_dir=$arg[1];
            $pdb_orig=$arg[1];
            $pdb_orig .= "/";
            $pdb_orig .= "$lu_str";
        }
        elsif (@arg[0] eq "-pdb-new")
        {
            $pdb_new_dir=$arg[1];
            $pdb_new=$arg[1];
            $pdb_new .= "/";
            $pdb_new .= "$lu_str";
        }
        elsif (@arg[0] eq "-xml-orig")
        {
            $xml_orig=$arg[1];
        }
        elsif (@arg[0] eq "-xml-new")
        {
            $xml_new_dir=$arg[1];
            $xml_new=$arg[1];
            $xml_new .= "/";
            $xml_new .= "$lu_str";
        }
        elsif (@arg[0] eq "-env")
        {
            &set_env_from_props($arg[1]);
        }
        elsif (@arg[0] eq "-list")
        {
            $test_list = $arg[1];
        }
        elsif (@arg[0] eq "-test")
        {
            $infile = $arg[1];
        }
        elsif (@arg[0] eq "-merge")
        {
             $merge_opt = 1;
        }
        else
        {
            print_usage();
            die "Incorrect command line";
        }
    }
}

# set_env_from_props
# infile - property file
#
# Read the properties file, of the form key=value
# Valid key values are :
#	POSE_EXE
#	POSE_PRC
#	POSE_PERL
#	TEST_HOME
#	PDB_ORIG
#	PDB_NEW
#	XML_ORIG
#	XML_NEW
# If a value is found the appropriate global variable is set.
#
# Sets these globals: pose_exe, pose_prc, xml_orig, xml_new_dir,
# xml_new, pdb_orig_dir, pdb_orig, pdb_new_dir, pdb_new.
#
sub set_env_from_props
{
    my $infile = $_[0];
    my $lu_str = &get_date_string();

    open(PROPSFILE, $infile) || die "Could not open properties file <$infile>";
    
    while (<PROPSFILE>)
    {
        chomp $_;
        my @arg = split('=', $_);
        @arg[0] =~ tr/a-z/A-Z/;
        my $len = @arg;
        if ($len != 2)
        {
            die "Malformed property in $arg[0]";
        }
        if (@arg[0] eq "POSE_EXE")
        {
            $pose_exe=$arg[1];
        }
        elsif (@arg[0] eq "POSE_PRC")
        {
            $pose_prc=$arg[1];
        }
        elsif (@arg[0] eq "PDB_ORIG")
        {
            $pdb_orig_dir=$arg[1];
            $pdb_orig=$arg[1];
            $pdb_orig .= "/";
            $pdb_orig .= "$lu_str";
        }
        elsif (@arg[0] eq "PDB_NEW")
        {
            $pdb_new_dir=$arg[1];
            $pdb_new=$arg[1];
            $pdb_new .= "/";
            $pdb_new .= "$lu_str";
        }
        elsif (@arg[0] eq "XML_ORIG")
        {
            $xml_orig=$arg[1];
        }
        elsif (@arg[0] eq "XML_NEW")
        {
            $xml_new_dir=$arg[1];
            $xml_new=$arg[1];
            $xml_new .= "/";
            $xml_new .= "$lu_str";
        }
        
    }
    close PROPSFILE;
}

# verify_env_options
#
# Verify that input options are correctly set.
# Assumes pose_exe, pose_prc, xml_orig, xml_new_dir,
# pdb_orig_dir, and pdb_new_dir are already set.
#
sub verify_env_options
{
    if (!-e "$pose_exe")
    {
        die "The pose executable cannot be found at $pose_exe.";
    }
    if (!-x $pose_exe)
    {
        die "$pose_exe exists but is not executable.";
    }
    
    if (!-e "$pose_prc")
    {
        die "The PRC directory specified as $pose_prc does not exist.";
    }
    if (!-d "$pose_prc")
    {
        die "The PRC location specified as $pose_prc exists, but is not a directory.";
    }
    
    if (!-e "$pdb_orig_dir")
    {
        die "The original PDB directory specified as $pdb_orig_dir does not exist.";
    }
    if (!-d "$pdb_orig_dir")
    {
        die "The original PDB directory specified as $pdb_orig_dir exists but is not a directory.";
    }
    
    if (!-e "$pdb_new_dir")
    {
        die "The new PDB directory specified as $pdb_new_dir does not exist.";
    }
    if (!-d "$pdb_new_dir")
    {
        die "The new PDB directory specified as $pdb_new_dir exists but is not a directory.";
    }
    
    if (!-e "$xml_orig")
    {
        die "The original Office document directory specified as $xml_orig does not exist.";
    }
    if (!-d "$xml_orig")
    {
        die "The original Office document location specified as $xml_orig exists but is not a directory.";
    }
    
    if (!-e "$xml_new_dir")
    {
        die "The new Office document directory specified as $xml_new_dir does not exist.";
    }
    if (!-d "$xml_new_dir")
    {
        die "The new Office document location specified as $xml_new_dir exists but is not a directory.";
    }
}

# verify_prcs_exist
# prcfile - the PRC file to check
#
# Verifies that the specified PRC file exists.
#
sub verify_prcs_exist
{
    my $prcfile = $_[0];

    if (!-e "$pose_prc/$prcfile")
    {
        die "The pose PRC directory ($pose_prc) is correct, but I can't find $prcfile there.";
    }
}

