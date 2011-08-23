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
# File Name: converterlib.pm
# Version  : 1.0
# Project  : XMerge
# Author   : Brian Cameron
# Date     : 5th Sept. 2001
#
# This script enters text at position x,y on screen.
#
# Parameter
#  x-coordinate
#  y-coordinate
#  Text to enter
#
##########################################################################

use EmRPC;  # EmRPC::OpenConnection, CloseConnection
use EmFunctions;
use EmUtils;

# Set global_debug flag
#
$global_debug   = $ENV{'ZENDEBUG'};
#$em_script_home = "/export/home/test/qadir/bin";
$em_script_home = $ENV{'EM_SCRIPT_HOME'};
#$qa_script_home = "/export/home/test/qadir/qa-new/bin";
    $qa_script_home = $ENV{'QA_SCRIPT_HOME'};
#
# CONVERT FUNCTIONS
#

# convert_to_pdb
# directory  - directory containing the xml-orig and pdb-orig 
#              subdirectories.
# file       - file to convert
# extension  - extension of file to convert (sxw or sxc)
# convert_to - what PDB format to convert into.
#
# Returns 0 if success, -1 otherwise.
#
# Converts file from XML to PDB
#
sub convert_to_pdb
{
  my $directory     = $_[0];
  my $file          = $_[1];
  my $extension     = $_[2];
  my $convert_to    = $_[3];
  my $pdb_directory = $_[4];
  my $rc            = 0;
  my $xmlfile       = "$directory/$file.$extension";
  my $pdbdir        = "$pdb_directory";

  &enter_func("convert_to_pdb");

  if (! -f "$xmlfile")
  {
    print "\nERROR, file $xmlfile does not exist\n";
    $rc = -1;
  }
  if (! -d "$pdbdir")
  {
    print "\nERROR, directory $directory/pdb-orig does not exist\n";
    $rc = -1;
  }

  if ($rc != -1)
  {
    if ("$convert_to" eq "application/x-minicalc")
    {
      # Move all files over.
      #
      my $i = 1;

      while (-f "$pdbdir/$file-Sheet$i.pdb")
      {
        my $pdbfile = "$pdbdir/$file-Sheet$i.pdb";

        print "\n";

        if (-f "$pdbfile.old")
        {
          print "Removing $pdbfile.old\n";
          `/bin/rm -f $pdbfile.old`;
        }

        print "Moving $pdbfile file to $pdbfile.old\n";
        `mv "$pdbfile" "$pdbfile.old"`;

        $i++;
      }
    }
    else
    {
      if (-f "$pdbdir/$file.pdb")
      {
        print "\n";

        if (-f "$pdbdir/$file.pdb.old")
        {
          print "Removing $pdbdir/$file.pdb.old\n";
          `/bin/rm -f $pdbdir/$file.pdb.old`;
        }

        print "Moving $pdbdir/$file.pdb file to $pdbdir/$file.pdb.old\n";
        `mv "$pdbdir/$file.pdb" "$pdbdir/$file.pdb.old"`
      }
    }

    &start_rd($extension, $convert_to, $xmlfile, "");

    if ("$convert_to" eq "application/x-minicalc")
    {
      # Must handle minicalc separately since it can
      # convert to multiple files with this file name
      # convention.
      #
      print "Moving $file-Sheet*.pdb files to $pdbdir\n";
      `mv $file-Sheet*.pdb $pdbdir`;
      `chmod 666 $pdbdir/$file-*.pdb`;
    }
    else
    {
      print "Moving $file.pdb file to $pdbdir\n";
      `mv $file.pdb $pdbdir`;
      `chmod 666 $pdbdir/$file.pdb`;
    }
  }

  &leave_func("convert_to_pdb");

  return $rc;
}

# convert_to_xml
# xmldir       - directory to contain the xml output.
# xmlorigdir   - directory to contain the xml input (used for merge)
# pdbfile      - file to convert
# convert_from - what PDB format to convert from.
# extension    - extension of file to convert (sxw or sxc)
# output       - output filename to create
# merge_opt    - 1 if convert and merge, 0 if convert only
#
# Returns 0 if success, -1 otherwise.
#
# Converts file from PDB to XML
#
sub convert_to_xml
{
  my $xmldir        = $_[0];
  my $xmlorigdir    = $_[1];
  my $pdbfile       = $_[2];
  my $convert_from  = $_[3];
  my $extension     = $_[4];
  my $output        = $_[5];
  my $merge_opt     = $_[6];
  my $rc            = 0;

  &enter_func("convert_to_xml");

  my @args = split(/ /,$pdbfile);

  for ($i=0;$i <= $#args; $i++)
  {
    if (! -f "@args[$i]")
    {
      print "\nERROR, file $pdbfile does not exist\n";
      $rc = -1;
    }
  }

  if (! -f "$xmlorigdir/$output.$extension")
  {
    print "\nERROR, file $xmlorigdir/$output.$extension does not exist\n";
    $rc = -1;
  }
  if (! -d "$xmldir")
  {
    print "\nERROR, directory $xmlorigdir does not exist\n";
    $rc = -1;
  }
  if (! -d "$xmlorigdir")
  {
    print "\nERROR, directory $xmldir does not exist\n";
    $rc = -1;
  }

  if ($rc != -1)
  {
    if ($merge_opt == 1)
    {
      print "Copying <$xmlorigdir/$output.$extension> to <$xmldir>\n";
      `cp $xmlorigdir/$output.$extension $xmldir/`;

      my $check_stamp = (stat("$xmldir/$output.$extension"))[9];

      &start_rd($convert_from, $extension, $pdbfile,
        "$xmldir/$output.$extension");

      
      # No need to move the file to the $xmldir since the merge
      # argument specifies the output file.
      
      my $check_stamp_update = (stat("$xmldir/$output.$extension"))[9];
      if ($check_stamp eq $check_stamp_update)
      {
        print "\nERROR, Problem while merging <$xmldir/$output.$extension>\n";
        `mv $xmldir/$output.$extension $xmldir/$output.$extension.err`;
      }
    }
    else
    {
      &start_rd($convert_from, $extension, $pdbfile, "");

      print "Moving $output.$extension to $xmldir\n";
      `mv $output.$extension $xmldir`;   
      `chmod 666 $xmldir/$output.$extension`;
    }
  }

  &leave_func("convert_to_xml");

  return $rc;
}

# start_rd
# from  - format to convert from
# to    - format to convert to
# file  - file to convert
# merge - merge filename ("" indicates convert-only with no merge)
# 
# converts file from/to the specified formats.
#
sub start_rd
{
    my $from       = $_[0];
    my $to         = $_[1];
    my $file       = $_[2];
    my $merge      = $_[3];

    print "\nConverting from $from to $to.\n";
    if ($global_debug)
    {
      &print_debug ("rd command is:\n");
    }

    if ($merge eq "")
    {
       &print_debug ("  $em_script_home/rd -from $from -to $to $file\n");
       print "\nConverting from $from to $to with no merge.\n";
      `$em_script_home/rd -from $from -to $to $file`;
    }
    else
    {
       &print_debug ("  $em_script_home/rd -from $from -to $to -merge $merge $file\n");
       print "\nConverting from $from to $to with merge.\n";
      `$em_script_home/rd -from $from -to $to -merge $merge $file`;
    }

    print "Done converting.\n\n";
}

#
# POSE INTERACTION FUNCTIONS
#

# open_connection
# display_debug - debug will be displayed if not 0
#
# Opens the connection to pose.
#
sub open_connection
{
  my $display_debug = $_[0];
  my $rc;

  EmRPC::OpenConnection(6415, "localhost");

  if ($display_debug && $global_debug)
  {
    print "\nPose Connection Opened\n";
  }
}

# close_connection
# display_debug - debug will be displayed if not 0
#
# Closes the connection to pose.
#
sub close_connection
{
  my $display_debug = $_[0];

  EmRPC::CloseConnection();

  if ($display_debug && $global_debug)
  {
    print "\nPose Connection Closed\n";
  }
}

# start_pose
# pose_exe  - name of pose executable.
# apps_load - The PRC files to load into pose, can be a comma
#             separated list.
# run_prog  - Program to run at startup. 
# timeout   - Timeout value to use when starting pose.
#
# Starts the Palm OS Emulator, loads PRC files, and starts
# a program.
#
sub start_pose
{
  my $pose_exe     = $_[0];
  my $sessionfile  = $ENV{'EM_SESSION_FILE'};
  my $romfile      = $ENV{'EM_ROM_FILE'};
  my $apps_load    = $_[1];
  my $run_prog     = $_[2];
  my $timeout      = $_[3];
  my $stay_in_loop = 1;
  my $address;
  my $title;
  my $form;
  my $label_id;
  my $num_objects;
  my $i;
  my $ii;
  my $rc = 1;

  my $pose_cmd  = "$pose_exe ";
  $pose_cmd    .= " -psf $sessionfile ";
  $pose_cmd    .= "-load_apps $apps_load ";
  $pose_cmd    .= "-run_app $run_prog";

# It is more effective to use the -psf argument to
# set these values.
#
#  $pose_cmd    .= -rom $romfile ";
#  $pose_cmd    .= "-ram_size 8192 ";
#  $pose_cmd    .= "-device PalmVx ";

  &enter_func("start_pose");

  if ($global_debug)
  {
    &print_debug("\n");
    &print_debug("pose command is:\n");
    &print_debug("  $pose_cmd\n");
  }

  print "\nLaunching pose...\n";
  system ("$pose_cmd &");

  # Give time for pose to get started...
  #
  for ($i=0; $i < $timeout; $i++)
  {
    $tmp = $i + 1;
    print "$tmp\n";

    # Do not use pose_sleep here
    #
    sleep(1);
  }

  # Verify pose started successfully, and fail otherwise...
  #
  $rc = &verify_pose(5);
  if ($rc != 0)
  {
     $stay_in_loop = 0;
  }
  else
  {
     # Sleep before opening the connection again, after testing in
     # the verify_pose function.
     #
     pose_sleep(2);
     &open_connection(1);
     print "\nChecking if the appropriate window is on screen...\n";
  }
  
  # Stop looping when the specified window has started.
  #
  for ($i=0; $i < $timeout && $stay_in_loop == 1; $i++)
  {
    $form = FrmGetActiveForm();
    $num_objects = FrmGetNumberOfObjects($form);

    for $ii (0..$num_objects - 1)
    {
      my ($object_type) = FrmGetObjectType($form, $ii);

      if ("$run_prog" eq "Quickword")
      {
        if ($object_type == frmTitleObj)
        {
          ($address, $title) = FrmGetTitle($form,);

          # Display count and title.
          #
          $tmp = $i + 1;
          print "$tmp - title is $title\n";

          if ("$title" eq "Quickword")
          {
            $stay_in_loop = 0;
            $rc = 0;
            last;
          }
        }
      }
      elsif ("$run_prog" eq "MiniCalc")
      {
        if ($object_type == frmLabelObj)
        {
          $label_id = FrmGetObjectId ($form, $ii);
          ($address, $label) = FrmGetLabel($form, $label_id);

          # Display count and label.
          #
          $tmp = $i + 1;
          print "$tmp - label is $label\n";
          if ("$label" =~ "Solutions In Hand")
          {
            $stay_in_loop = 0;
            $rc = 0;
            last;
          }
        }
      }
    }

    # Do not use pose_sleep here
    #
    sleep(1);
  }

  # Do not use pose_sleep here
  #
  sleep(1);

  &leave_func("start_pose");
  return($rc);
}

# kill_pose
#
# Kills all pose processes
#
sub kill_pose
{
  if ($global_debug)
  {
     print "Stopping pose process...\n";
  }

  `pkill pose`;
}

# verify_pose
# timeout - timeout to wait for pose
#
#  Tries to do a connect/close to Pose to see if
#  it is working okay.
#
sub verify_pose
{
  my $timeout = $_[0];
  my $rc = 0;

  $rc = system("$em_script_home/verify_sane.pl $timeout");
  return $rc;
}

# db_export
# dbname - Name of database to export
#
#  Exports a palmdb file to /tmp
#
sub db_export
{
  my $dbname = $_[0];

  &enter_func("db_export");
  print "\nExporting PDB file <$dbname> from pose\n";
  &pose_tap_pen(22, 20, 2);
  &pose_tap_pen (15, 85, 2);
  &enter_string($dbname, 1);
  &pose_tap_pen (15, 126, 1);
  &enter_string("/tmp/", 1);
  &pose_tap_button("OK", 4);
  &tap_applications(3);
  print "Export of PDB file <$dbname> completed.\n";
  &leave_func("db_export");
}

#
# QUICKWORD SPECIFIC
#

# start_quickword
#
# Assuming pose was launched with the -run_app flag to launch
# QuickWord on startup, this starts up QuickWord with the first
# file in the list and turns off write-protect.
#
sub start_quickword
{
  &enter_func("start_quickword");

  # This will open the first file in the list.
  # Assuming this will always be the case.
  #
  &pose_tap_pen(20, 18, 1);
  &quickword_press_write_protect();

  &leave_func("start_quickword");
}

# quickword_press_write_protect
#
# Useful function for pressing the write protect button
# to allow changes to be made.
#
sub quickword_press_write_protect
{
  &enter_func("quickword_press_write_protect");

  my ($form) = FrmGetActiveForm();
  my ($num_objects) = FrmGetNumberOfObjects($form);

  for $ii (0..$num_objects - 1)
  {
    my ($object_type) = FrmGetObjectType($form, $ii);

    # The write protect button is the only frmGadgetObj
    # on the QuickWord screen.
    #
    if ($object_type == frmGadgetObj)
    {
       my (%bounds) = FrmGetObjectBounds($form, $ii);

       if ($global_debug)
       {
          &print_debug("  Found QuickWord WriteProtect button\n");
          &print_debug("    left   = $bounds{left}\n");
          &print_debug("    right  = $bounds{right}\n");
          &print_debug("    top    = $bounds{top}\n");
          &print_debug("    bottom = $bounds{bottom}\n");
       }
       
       # For some reason, the tapping of the write-protect button
       # doesn't work unless you tap somewhere else first.
       #
       &pose_sleep(1);
       &pose_tap_pen($bounds{left} + 2, $bounds{top} + 2, 1);
       last;
    }
  }

  &leave_func("quickword_press_write_protect");
}

# quickword_find_replace
# from_string - string to replace
# to_string - string to replace with
#
# Uses QuickWord's find/replace utility to replace
# one string with another.
# 
sub quickword_find_replace
{
  my $from_string = $_[0];
  my $to_string   = $_[1];

  &enter_func("quickword_find_replace");
 
  # Move cursor to beginning...
  #
  &quickword_tap_at_top(1);

  # Move to "Find" field:
  # Triple-click to highlight all the text in the field,
  # so it is removed when the string is entered...
  #
  &pose_tap_button("Find", 2);
  &pose_tap_pen(50, 100, 0);
  &pose_tap_pen(50, 100, 0);
  &pose_tap_pen(50, 100, 1);

  # sleep for 2 seconds to avoid double click after moving
  # to replace field
  #
  &enter_string("$from_string", 2);

  # Move to "Replace" field:
  # Triple-click to highlight all the text in the field,
  # so it is removed when the string is entered...
  #
  &pose_tap_pen(50, 120, 0);
  &pose_tap_pen(50, 120, 0);
  &pose_tap_pen(50, 120, 1);
  &enter_string("$to_string", 1);

  # Do find, then replace...
  #
  &pose_tap_button("Find", 1);
  &pose_tap_button("Replace", 1);
  &pose_tap_button("Cancel", 1);

  &leave_func("quickword_find_replace");
}

# quickword_tap_at_top
# secs - seconds to sleep after the tap
#
# Tap's at the top of the QuickWord document.
#
sub quickword_tap_at_top
{
  my $secs = $_[0];

  &enter_func("quickword_tap_at_top");

  # Sleep for a second to avoid any double-clicks
  # from happening.
  #
  &pose_sleep(1);

  &pose_tap_pen(0, 15, $secs);
  &leave_func("quickword_tap_at_top");
}

# Saves file and returns to the Application list.
#
sub close_quickword
{
  &enter_func("close_quickword");

  &pose_tap_button("Done", 2);
  &tap_applications(2);

  &leave_func("close_quickword");
}

#
# MINICALC SPECIFIC
#

# start_minicalc
#
# Assuming pose was launched with the -run_app flag to launch
# Minicalc on startup, this starts up Minicalc with the first
# file in the list.
#
sub start_minicalc
{
  &enter_func("start_minicalc");
  &pose_tap_button("OK", 1);

  # For now just tap on the first spreadsheet.  Add support
  # for multiple sheets later.
  #
  &pose_tap_pen(10, 40, 5);

  &leave_func("start_minicalc");
}

# close_minicalc
#
# Returns to the Application list (no need to save).
#
sub close_minicalc
{
  &enter_func("close_minicalc");
  &tap_applications(3);
  &leave_func("close_minicalc");
}

# minicalc_enter_cell
# row - row to enter value, starting with 1
# col - column to enter value, starting with 1
# val - value to enter
#
# Only valid for minicalc.
#
# This only works if the val passed in has a '\n' at the
# end.
#
sub minicalc_enter_cell
{
  my $row = $_[0];
  my $col = $_[1];
  my $val = $_[2];
  my $i;
  my $j;

  &enter_func("minicalc_enter_cell");

  if ($global_debug)
  {
    &print_debug ("  tapping to cell row=<$row> col=<$col>\n");
  }

  # Tap pen on home button to start with row=1, col=A
  # at top left.  
  #
  pose_tap_pen(1, 1, 3);

  # Now the cell should be in the top-left corner,
  # so click there.  However we must first click 
  # in another cell or pose doesn't acknowledge the
  # click.
  #
  # pose_tap_pen(120, 95, 1);
  # pose_tap_pen(21, 9, 1);

  # Click the down button once for each row.
  # Must pause 3 seconds each time, otherwise MiniCalc
  # will not keep up.
  #
  for ($i=0; $i < $row; $i++)
  {
    if ($global_debug)
    {
      &print_debug ("  Typing carrage return to go down\n");
    }
    enter_string("\n", 1);
  }

  # Click the right button once for each col.
  # Must pause 3 seconds each time, otherwise MiniCalc
  # will not keep up.
  #
  for ($i=0; $i < $col; $i++)
  {
    if ($global_debug)
    {
      &print_debug ("  Typing tab to go right\n");
    }

    enter_string("\t", 1);
  }

  # enter string
  #
  &enter_string($val, 1);

  &leave_func("minicalc_enter_cell");
}

#
# GENERIC UTILIIES (pose)
#

# tap_applications
# secs - seconds to sleep after the tap
#
# taps pen on the Applications button.
#
sub tap_applications
{
  my $secs = $_[0];

  &enter_func("tap_applications");

  &pose_tap_pen(15, 170, 1);
  &pose_tap_pen(155, 10, 1);
  &pose_tap_pen(155, 10, $secs);

  &leave_func("tap_applications");
}

# enter_string_at_location
# x           - x-location to enter string
# y           - y-location to enter string
# in_string   - string to enter
# application - appliation (QUICKWORD or MINICALC)
#
# Enters a string at the specified x,y position.
#
sub enter_string_at_location
{
  my $x_val       = $_[0];
  my $y_val       = $_[1];
  my $in_string   = $_[2];
  my $application = $_[3];
  my $x;
  my $y;

  &enter_func("enter_string_at_location");

  $x = $x_val;
  $y = $y_val;

  if ($application eq "QUICKWORD")
  {
     # Allow users to specify TOP/BOTTOM/LEFT/RIGHT
     # for QuickWord.
     #
     if ($y_val eq "TOP")
     {
        if ($global_debug)
        {
           &print_debug("  Converting TOP to 15\n");
        }

        $y = 15;
     }
     if ($y_val eq "BOTTOM")
     {
        if ($global_debug)
        {
           &print_debug("  Converting BOTTOM to 144\n");
        }

        $y = 144;
     }
     if ($x_val eq "LEFT")
     {
        if ($global_debug)
        {
           &print_debug("  Converting LEFT to 0\n");
        }

        $x = 0;
     }
     if ($x_val eq "RIGHT")
     {
        if ($global_debug)
        {
           &print_debug("  Converting RIGHT to 152\n");
        }

        $x = 152;
     }
  }

  # Just to make sure the offset isn't outside the 
  # proper area.
  #
  if ($x >= 100)
  {
    $offset = -2;
  }
  else
  {
    $offset = 2;
  }

  &off_tap_pen($x, $y, $offset);
  &enter_string($in_string, 1);

  &leave_func("enter_string_at_location");
}

# off_tap_pen
# x      - x-location to tap
# y      - y-location to tap
# offset - x-offset to use for first tap.
# 
# For some reason, pose does not register a single
# pen tap if the last single pen tap was also
# at the same x,y coordinate (even if the last tap 
# was a while ago).  So this function does two
# slightly different pen taps to ensure then pen
# tap happens.
#
sub off_tap_pen
{
  my $x      = $_[0];
  my $y      = $_[1];
  my $offset = $_[2];

  &enter_func("off_tap_pen");

  # sleep for 2 seconds to avoid double-click.
  #
  &pose_tap_pen_hard($x + $offset, $y, 2);
  &pose_tap_pen_hard($x, $y, 1);

  &leave_func("off_tap_pen");
}

# enter_string
# in_string - string to enter
# secs - seconds to sleep after entering the string
#
# Enters a string
#
sub enter_string
{
  my $in_string = $_[0];
  my $secs = $_[1];
  my $j;

  &enter_func("enter_string");
  
  if ($global_debug)
  {
     # Display in_string so \n and \t values
     # show up as normal ASCII.
     #
     if ($in_string eq "\n")
     {
        &print_debug("  Entering string     : <\\n>\n");
     }
     elsif ($in_string eq "\t")
     {
        &print_debug("  Entering string     : <\\t>\n");
     }
     else
     {
        &print_debug("  Entering string     : <$in_string>\n");
     }
  }

  # Replace "\n" with real carrage returns.
  #
  my $string_val = $in_string;
  $string_val =~ s#\\n#\n#g;

  # Replace "\t" with a real tab.
  #
  $string_val =~ s#\\t#\t#g;

  # Convert string to ASCII numeric values
  #
  my @array = unpack("C*", $string_val);

  # Enter string one key at a time.
  #
  for ($j=0; $j <= $#array; $j++)
  {
     $queue_size = EnterKey($array[$j], 0, 0);
  }

  if ($secs > 0)
  {
     pose_sleep($secs);
  }

  &leave_func("enter_string");
}

#
# GENERIC UTILIIES (non pose)
#

# get_date_string
#
# Returns a timestampe string in yyyymmddHHMM format, where:
#   yyyy = year
#   mm   = month
#   dd   = day
#   HH   = hour
#   MM   = minute
#
# This sort of datestamp is used to create the output directory
# names, so it used in various places.
#
sub get_date_string
{
    my $cur_secs  = time;
    my @lu = localtime $cur_secs;
    my $lu_secs  = $lu[1];
    my $lu_hours = $lu[2];
    my $lu_day   = $lu[3];
    my $lu_mon   = $lu[4] + 1;
    my $lu_year  = $lu[5] + 1900;
    my $lu_str   = $lu_year;

    if ($lu_mon < 10)
    {
       $lu_str .= "0";
    }
    $lu_str .= $lu_mon;

    if ($lu_day < 10)
    {
       $lu_str .= "0";
    }
    $lu_str .= $lu_day;

    if ($lu_hours < 10)
    {
       $lu_str .= "0";
    }
    $lu_str .= $lu_hours;

    if ($lu_secs < 10)
    {
       $lu_str .= "0";
    }
    $lu_str .= $lu_secs;

    return $lu_str;
}

#
# DEBUG FUNCTIONS - Wrapper functions
#

# pose_tap_pen
# x    - x-position of pen tap
# y    - y-position of pen tap
# secs - seconds to sleep after the tap
#
# Taps pen at specified position and displays debug info
#
sub pose_tap_pen
{
  my $x           = $_[0];
  my $y           = $_[1];
  my $secs        = $_[2];

  if ($global_debug)
  {
    &print_debug("  Tapping pen at      : $x,$y\n");
  }

  TapPen($x, $y);

  if ($secs > 0)
  {
     pose_sleep($secs);
  }
}

# pose_tap_pen_hard
# x    - x-position of pen tap
# y    - y-position of pen tap
# secs - seconds to sleep after the tap
#
# Taps pen at specified position and displays debug info
# This function works more effectively in situations where
# pose_tap_pen is flakey.  This function is not good for
# double/triple click situations since it is slow.
#
sub pose_tap_pen_hard
{
  my $x           = $_[0];
  my $y           = $_[1];
  my $secs        = $_[2];

  if ($global_debug)
  {
    &print_debug("  Tapping pen hard at : $x,$y\n");
  }

  `$qa_script_home/tappen.pl $x $y`;

  if ($secs > 0)
  {
     pose_sleep($secs);
  }
}

# pose_tap_button
# button - button to press
# secs - seconds to sleep after the button press
#
# Presses specified button and displays debug info
#
sub pose_tap_button
{
  my $button = $_[0];
  my $secs = $_[1];

  if ($global_debug)
  {
    &print_debug("  Tapping button      : $button\n");
  }

  TapButton($button);

  if ($secs > 0)
  {
     pose_sleep($secs);
  }
}

# pose_sleep
# secs - seconds to sleep
#
# Sleeps the specified amount of time and displays debug info
#
sub pose_sleep
{
  my $secs = $_[0];

  if ($global_debug)
  {
    &print_debug("  Sleeping            : $secs seconds\n");
  }

  sleep($secs);
}

# enter_func
# func - function name
#
# Displays debug info about entering specified function.
#
sub enter_func
{
  my $func = $_[0];

  if ($global_debug)
  {
    &print_debug("Function enter        : $func\n");
  }
}

# leave_func
# func - function name
#
# Displays debug info about leaving specified function.
#
sub leave_func
{
  my $func = $_[0];

  if ($global_debug)
  {
    &print_debug("Function exit         : $func\n");
  }
}

# print_debug
# string - string to print
#
# Displays debug message with a # at the beginning of the line.
#
sub print_debug
{
   my $string = $_[0];

   print "# $string";
}

1;

