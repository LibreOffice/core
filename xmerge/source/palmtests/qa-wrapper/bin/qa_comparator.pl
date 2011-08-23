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

$compare_home = $ENV{'QA_COMPARATOR_HOME'};

if ($ENV{'CLASSPATH'})
{
   $classpath_val = "$compare_home:$ENV{'CLASSPATH'}";
}
else
{
   $classpath_val = "$compare_home";
}

print "classpath is $classpath_val\n";

$list_file="";
$orig_dir="";
$new_dir="";
$diff_type="";

####### BEGIN MAIN ##############
$cmdline_len = @ARGV;
if ($cmdline_len <= 0)
{
    print_usage();
    exit (0);
}

process_cmdline(@ARGV);
print_env();
open (LOGFILE, ">$logfile") || die "Cannot open log file $logfile";
if ($test_list ne "")
{
    open (TESTLIST, $test_list) || die "Couldn't open diff list file $test_list";

    while (<TESTLIST>)
    {
        chomp $_;
        process_diff(get_file_title($_));
    }
}
close TESTLIST;
close LOGFILE;

####### END MAIN ##############

sub process_diff
{
#	$_[0] =~ tr/A-Z/a-z/;

    # chdir to the output directory so the temporary files created by
    # the java programs are put in the right place.
    #
    chdir ($xml_new);

    if ($diff_type eq "xml")
    {
        # Ugly hack, probably a way to tell xerces directly that the dtd's
        # are in $compare_home/dtd.
        #
        `cp $compare_home/dtd/* $xml_new`;

#		$cmd = "java -classpath $classpath_val XmlWrapper $xml_orig/$_[0].sxw $xml_new/$_[0].sxw";
        $cmd = "java -classpath $classpath_val XmlWrapper $xml_orig/$_[0] $xml_new/$_[0]";
        print "Executing: $cmd\n";
        $val = system($cmd)/256;
        if ($val == 2)
        {
#			print LOGFILE "$_[0]|TRUE|$xml_orig/$_[0].sxw|$xml_new/$_[0].sxw\n";
            print LOGFILE "$_[0]|TRUE|$xml_orig/$_[0]|$xml_new/$_[0]\n";
        }
        elsif($val == 3)
        {
#			print LOGFILE "$_[0]|FALSE|$xml_orig/$_[0].sxw|$xml_new/$_[0].sxw\n";
            print LOGFILE "$_[0]|FALSE|$xml_orig/$_[0]|$xml_new/$_[0]\n";
        }
        else
        {
#			print LOGFILE "$_[0]|ERROR|$xml_orig/$_[0].sxw|$xml_new/$_[0].sxw\n";
            print LOGFILE "$_[0]|ERROR|$xml_orig/$_[0]|$xml_new/$_[0]\n";
        }
    }
    elsif ($diff_type eq "pdb")
    {
#		$cmd = "java -classpath $classpath_val SimplePdbCompare $pdb_orig/$_[0].pdb $pdb_new/$_[0].pdb\n";
        $cmd = "java -classpath $classpath_val SimplePdbCompare $pdb_orig/$_[0] $pdb_new/$_[0]\n";
        print "Executing: $cmd\n";
        $val = system($cmd)/256;
        if ($val == 2)
        {
#			print LOGFILE "$_[0]|TRUE|$pdb_orig/$_[0].pdb|$pdb_new/$_[0].pdb\n";
            print LOGFILE "$_[0]|TRUE|$pdb_orig/$_[0]|$pdb_new/$_[0]\n";
        }
        elsif($val == 3)
        {
#			print LOGFILE "$_[0]|FALSE|$pdb_orig/$_[0].pdb|$pdb_new/$_[0].pdb\n";
            print LOGFILE "$_[0]|FALSE|$pdb_orig/$_[0]|$pdb_new/$_[0]\n";
        }
        else
        {
#			print LOGFILE "$_[0]|ERROR|$pdb_orig/$_[0].pdb|$pdb_new/$_[0].pdb\n";
            print LOGFILE "$_[0]|ERROR|$pdb_orig/$_[0]|$pdb_new/$_[0]\n";
        }
    }
    else
    {
        die "Don't understand test type of $diff_type.";
    }
}

sub process_cmdline
{
    foreach $i (@_)
    {
        @arg= split('=', $i);
        @arg[0] =~ tr/A-Z/a-z/;
        
        if (@arg[0] eq "-pdb-orig")
        {
            $pdb_orig=$arg[1];
        }
        elsif (@arg[0] eq "-pdb-new")
        {
            $pdb_new=$arg[1];
        }
        elsif (@arg[0] eq "-xml-orig")
        {
            $xml_orig=$arg[1];
        }
        elsif (@arg[0] eq "-xml-new")
        {
            $xml_new=$arg[1];
        }
        elsif (@arg[0] eq "-env")
        {
            set_env_from_props($arg[1]);
        }
        elsif (@arg[0] eq "-list")
        {
            $test_list = $arg[1];
        }
        elsif (@arg[0] eq "-one")
        {
            $infile = $arg[1];
        }
        elsif (@arg[0] eq "-type")
        {
            $diff_type = $arg[1];
            chomp $diff_type;
        }
        elsif (@arg[0] eq "-log")
        {
            $logfile = $arg[1];
        }
        else
        {
            print_usage();
            die "Incorrect command line. Don't understand $i";
        }
    }
}

sub set_env_from_props
{
    open(PROPSFILE, $_[0]) || die "Could not open properties file";
    
    while (<PROPSFILE>)
    {
        chomp $_;
        @arg = split('=', $_);
        @arg[0] =~ tr/a-z/A-Z/;
        $len = @arg;
        if ($len != 2)
        {
            die "Malformed property in $ARGV[0]";
        }

        if (@arg[0] eq "PDB_ORIG")
        {
            $pdb_orig=$arg[1];
        }
        elsif (@arg[0] eq "PDB_NEW")
        {
            $pdb_new=$arg[1];
        }
        elsif (@arg[0] eq "XML_ORIG")
        {
            $xml_orig=$arg[1];
        }
        elsif (@arg[0] eq "XML_NEW")
        {
            $xml_new=$arg[1];
        }
        
    }
    close PROPSFILE;
}

sub print_usage
{
    print "Usage : compartor.pl - compare Office or pdb files\n";
    print "\t-one=<file> :\t\t individual test case file to run\n";
    print "\t-list=<file> :\t\t list of test case files\n"; 
    print "\t-env=<file> :\t\t Properites like file defining env\n"; 
    print "\t-pdb-orig=<path> :\t directory to hold original pdb files\n";
    print "\t-pdb-new=<path> :\t directory to hold new pdb files\n";
    print "\t-xml-orig=<path> :\t directory to hold original office documents\n";
    print "\t-xml-new=<path> :\t directory to hold new office documents\n";
    print "\t-type=<xml|pdb> :\t Invokes the merge option when converting\n";
    print "\t-log=<logfile> :\t Save results to logfile.\n";
}

sub print_env
{
    print "Using the following environment:\n";
    print "\tPDB_ORIG  = $pdb_orig\n";
    print "\tPDB_NEW   = $pdb_new\n";
    print "\tXML_ORIG  = $xml_orig\n";
    print "\tXML_NEW   = $xml_new\n\n";
}

sub get_file_title
{
    @paths = split('\/', $_[0]);
    $len = @paths;
    return @paths[$len-1];
#	@names = split('\.', @paths[$len-1]);
#	return $names[0];
}
