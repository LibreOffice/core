#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



package installer::packagepool;

use Digest::MD5;
use installer::exiter;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::worker;

######################################################
# Checking the md5sum of a file
######################################################

sub get_md5sum
{
    my ($filename) = @_;

    open(FILE, "<$filename") or die "ERROR: Can't open $filename for creating file hash";
    binmode(FILE);
    my $digest = Digest::MD5->new->addfile(*FILE)->hexdigest;
    close(FILE);

    return $digest;
}

####################################################
# Setting a unique sessionid to identify this
# packaging process.
####################################################

sub set_sessionid
{
    my $pid = $$;       # process id
    my $timer = time(); # time
    $installer::globals::sessionid = $pid . $timer;
    $installer::globals::sessionidset = 1;
    $installer::logger::Lang->print("\n");
    $installer::logger::Lang->print("Pool: Setting session id: $installer::globals::sessionid.\n");
}

####################################################
# Setting and creating pool path.
####################################################

sub set_pool_path
{
    $installer::globals::unpackpath =~ s/\Q$installer::globals::separator\E\s*$//;  # removing ending slashes and backslashes
    $installer::globals::poolpath = $installer::globals::unpackpath . $installer::globals::separator . "pool_" . $installer::globals::packageformat;
    installer::systemactions::create_directory($installer::globals::poolpath);
    $installer::globals::poolpathset = 1;
}

####################################################
# Comparing the content of two epm files.
####################################################

sub compare_epm_content
{
    my ($oldcontent, $newcontent) = @_;

    my $identical = 1;
    my $diffinfo = "";

    # Removing empty lines and files from $newcontent

    my @newlocalcontent = ();
    for ( my $i = 0; $i <= $#{$newcontent}; $i++ )
    {
        if ( ${$newcontent}[$i] =~ /^\s*$/ ) { next; } # Removing empty lines from $newcontent. Empty lines are also not included into pcf file, from where $oldcontent was read.
        if ( ${$newcontent}[$i] =~ /^\s*f\s+/ ) { next; } # Ignoring files, they can contain temporary pathes
        if (( ${$newcontent}[$i] =~ /^\s*%readme\s+/ ) || ( ${$newcontent}[$i] =~ /^\s*%license\s+/ )) { next; } # ignoring license and readme (language specific!)
        my $oneline = ${$newcontent}[$i];
        $oneline =~ s/\s*$//; # Removing line ends. Also not included in old epm file, that is read from pcf file.
        push(@newlocalcontent, $oneline);
    }

    my $oldmember = $#{$oldcontent} + 1;
    my $newmember = $#newlocalcontent + 1;

    # comparing the count
    if ( $oldmember != $newmember )
    {
        $identical = 0;
        $installer::logger::Info->print("\n");
        $installer::logger::Info->print("...... changed length of EPM file\n");
        $diffinfo = "Pool: EPM, different line count: old epm file: $oldmember, new epm file: $newmember\n";
        push(@installer::globals::epmdifflist, $diffinfo);
    }

    # comparing the content line for line, so the order must not change

    if ( $identical )
    {
        for ( my $i = 0; $i <= $#{$oldcontent}; $i++ )
        {
            if ( ${$oldcontent}[$i] ne $newlocalcontent[$i] )
            {
                $identical = 0;
                my $line = $i + 1;
                $installer::logger::Info->print("\n");
                $installer::logger::Info->print("...... different content in EPM file\n");
                $diffinfo = "Pool: EPM, line $line changed from \"${$oldcontent}[$i]\" to \"$newlocalcontent[$i]\".\n";
                push(@installer::globals::epmdifflist, $diffinfo);
                last;
            }
        }
    }

    return $identical;
}

####################################################
# Comparing the content of two pcf files.
####################################################

sub compare_package_content
{
    my ($oldcontent, $newcontent) = @_;

    my $identical = 1;
    my $infoline = "";

    my $oldmember = scalar keys %{$oldcontent};
    my $newmember = scalar keys %{$newcontent};

    # comparing the count

    if ( $oldmember != $newmember )
    {
        # Logging the difference
        $identical = 0;
        $installer::logger::Info->print("\n");
        $installer::logger::Info->printf("...... different number of files in packages. New number: %s, old number: %s\n", $newmember, $oldmember);
        $infoline = "Different number of files in packages. New number: $newmember, old number: $oldmember\n";
        push(@installer::globals::pcfdiffcomment, $infoline);
    }

    # comparing the keys

    if ( $identical )
    {
        my $first = 1;
        foreach my $dest ( keys %{$newcontent} )
        {
            if ( ! exists($oldcontent->{$dest}) )
            {
                $identical = 0;
                $installer::logger::Info->print("\n") if $first;
                $installer::logger::Info->printf("...... file only in one package (A): %s\n", $dest);
                $infoline = "File only in existing pool package: $dest\n";
                push(@installer::globals::pcfdiffcomment, $infoline);
                $first = 0;
            }
        }

        # collecting all differences
        if ( ! $identical )
        {
            foreach my $dest ( keys %{$oldcontent} )
            {
                if ( ! exists($newcontent->{$dest}) )
                {
                    $identical = 0;
                    $installer::logger::Info->print("\n") if $first;
                    $installer::logger::Info->printf("...... file only in one package (B): %s\n", $dest);
                    $infoline = "File only in new package: $dest\n";
                    push(@installer::globals::pcfdiffcomment, $infoline);
                    $first = 0;
                }
            }
        }
    }

    # comparing the checksum

    if ( $identical )
    {
        my $first = 1;

        foreach my $dest ( keys %{$newcontent} )
        {
            if ( $newcontent->{$dest}->{'md5sum'} ne $oldcontent->{$dest}->{'md5sum'} )
            {
                $identical = 0;
                if ( $first == 1 )
                {
                    $installer::logger::Info->print("\n");
                    $first = 0;
                }
                $installer::globals::pcfdifflist{$dest} = 1;
                $installer::logger::Info->printf("...... different file: %s\n", $dest);
                # last;
            }

            if ( $installer::globals::iswindowsbuild )
            {
                if ( $newcontent->{$dest}->{'uniquename'} ne $oldcontent->{$dest}->{'uniquename'} )
                {
                    $identical = 0;
                    $installer::globals::pcfdifflist{$dest} = 1;
                    $installer::logger::Info->print("\n");
                    $installer::logger::Info->printf("...... different file: %s", $dest);
                    # last;
                }
            }
        }
    }

    return $identical;
}

####################################################
# Calculating content of pcf file.
####################################################

sub calculate_current_content
{
    my ($filesarray, $packagename) = @_;

    $installer::logger::Lang->print("\n");
    $installer::logger::Lang->add_timestamp("Calculating content for package content file ($packagename), start");

    my %globalcontent = ();

    for ( my $i = 0; $i <= $#{$filesarray}; $i++ )
    {
        my %onefilehash = ();

        my $onefile = ${$filesarray}[$i];
        if ( ! $onefile->{'sourcepath'} ) { installer::exiter::exit_program("ERROR: No sourcepath found for file $onefile->{'gid'}", "calculate_current_content");  }
        my $source = $onefile->{'sourcepath'};
        if ( $onefile->{'zipfilesource'} ) { $source = $onefile->{'zipfilesource'}; }
        if ( ! -f $source ) { installer::exiter::exit_program("ERROR: Sourcefile not found: $source ($onefile->{'gid'})", "calculate_current_content"); }

        # For Windows the unique name inside the cabinet file also has to be saved
        my $uniquename = "";
        if ( $installer::globals::iswindowsbuild ) { $uniquename = $onefile->{'uniquename'};}

        my $destination = $onefile->{'destination'};
        my $checksum = get_md5sum($source);

        $onefilehash{'md5sum'} = $checksum;
        $onefilehash{'uniquename'} = $uniquename;

        if ( exists($globalcontent{$destination}) ) { installer::exiter::exit_program("ERROR: Destination not unique: $destination ($onefile->{'gid'})", "calculate_current_content"); }
        $globalcontent{$destination} = \%onefilehash;
    }

    $installer::logger::Lang->print("\n");
    $installer::logger::Lang->add_timestamp("Calculating content for package content file ($packagename), start");

    return \%globalcontent;
}

####################################################
# Writing pcf file.
####################################################

sub create_pcfcontent_file
{
    my ($realpackagename, $md5sum, $filesize, $fullpackagename, $pkgversion, $epmfilecontent, $pcffilename) = @_;

    my @content = ();
    my $oneline = "PackageName: $realpackagename\n";
    push(@content, $oneline);

    $oneline = "md5sum: $md5sum\n";
    push(@content, $oneline);

    $oneline = "FileSize: $filesize\n";
    push(@content, $oneline);

    $oneline = "FullPackageName: $fullpackagename\n";
    push(@content, $oneline);

    $oneline = "PkgVersion: $pkgversion\n";
    push(@content, $oneline);

    foreach my $dest (keys %{$installer::globals::newpcfcontent} )
    {
        $oneline = "Files:\t$dest\t$installer::globals::newpcfcontent->{$dest}->{'md5sum'}\t$installer::globals::newpcfcontent->{$dest}->{'uniquename'}\n";
        push(@content, $oneline);
    }

    for ( my $i = 0; $i <= $#{$epmfilecontent}; $i++ )
    {
        if ( ${$epmfilecontent}[$i] =~ /^\s*$/ ) { next; } # avoiding empty lines
        if ( ${$epmfilecontent}[$i] =~ /^\s*f\s+/ ) { next; } # ignoring files, because they can contain temporary pathes
        if (( ${$epmfilecontent}[$i] =~ /^\s*%readme\s+/ ) || ( ${$epmfilecontent}[$i] =~ /^\s*%license\s+/ )) { next; } # ignoring license and readme (language specific!)
        $oneline = "EPM:\t${$epmfilecontent}[$i]";
        push(@content, $oneline);
    }

    installer::files::save_file($pcffilename, \@content);
}

#######################################################
# Reading the content of the package content file.
#######################################################

sub read_pcf_content
{
    my ($pcffilename) = @_;

    my %allcontent = ();
    my @epmfile = ();
    my $realpackagename = "";

    my $content = installer::files::read_file($pcffilename);

    for ( my $i = 0; $i <= $#{$content}; $i++ )
    {
        my $line = ${$content}[$i];

        if ( $line =~ /^\s*PackageName\:\s*(.*?)\s*$/ )
        {
            $realpackagename = $1;
            $installer::globals::xpdpackageinfo{'RealPackageName'} = $realpackagename;
            next;
        }

        if ( $line =~ /^\s*FullPackageName\:\s*(.*?)\s*$/ )
        {
            $installer::globals::xpdpackageinfo{'FullPackageName'} = $1;
            next;
        }

        if ( $line =~ /^\s*FileSize\:\s*(.*?)\s*$/ )
        {
            $installer::globals::xpdpackageinfo{'FileSize'} = $1;
            next;
        }

        if ( $line =~ /^\s*PkgVersion\:\s*(.*?)\s*$/ )
        {
            $installer::globals::xpdpackageinfo{'PkgVersion'} = $1;
            next;
        }

        if ( $line =~ /^\s*md5sum\:\s*(.*?)\s*$/ )
        {
            $installer::globals::xpdpackageinfo{'md5sum'} = $1;
            next;
        }

        if ( $line =~ /^\s*Files:\t(.+?)\t(.+?)\t(.*?)\s*$/ )
        {
            my $destination = $1;
            my $checksum = $2;
            my $uniquename = $3;

            my %onefilehash = ();
            $onefilehash{'md5sum'} = $checksum;
            $onefilehash{'uniquename'} = $uniquename;

            $allcontent{$destination} = \%onefilehash;
            next;
        }

        if ( $line =~ /^\s*EPM:\t(.*?)\s*$/ )    # A line can be empty in epm file
        {
            my $epmcontent = $1;
            push(@epmfile, $epmcontent);
            next;
        }
    }

    if ( $realpackagename eq "" ) { installer::exiter::exit_program("ERROR: Real package name not found in pcf file: \"$pcffilename\"", "read_pcf_content"); }

    return ($realpackagename, \%allcontent, \@epmfile);
}

####################################################
# Checking, if a specific package can be
# created at the moment.
####################################################

sub check_package_availability
{
    my ($packagename) = @_;

    my $package_is_available = 1;

    my $checkfilename = $installer::globals::poolpath . $installer::globals::separator . $packagename . ".pcf.check";
    my $lockfilename = $installer::globals::poolpath . $installer::globals::separator . $packagename . ".pcf.lock";

    if (( -f $checkfilename ) || ( -f $lockfilename )) { $package_is_available = 0; }

    return $package_is_available;
}

####################################################
# Check, if the existence of the check or lock
# file requires an exit of packaging process.
####################################################

sub check_pool_exit
{
    my ( $lockfilename, $timecounter ) = @_;

    # How old is this lock file?
    my $timeage = installer::logger::get_file_age($lockfilename);

    # if ( $timeage > 1800 ) # file is older than half an hour
    if ( $timeage > 3600 ) # file is older than an hour
    {
        my $timestring = installer::logger::convert_timestring($timeage);
        my $infoline = "\nPool: Attention: \"$lockfilename\" is too old ($timestring). Removing file!\n";
        $installer::logger::Info->print("\n");
        $installer::logger::Info->printf("... %s", $infoline);
        $installer::logger::Lang->print("\n");
        $installer::logger::Lang->print($infoline);
        unlink $lockfilename;
        # installer::exiter::exit_program("ERROR: Waiting too long for removal of lock file \"$lockfilename\"", "check_pool_exit (packagepool)");
    }
    else
    {
        my $filecontent = installer::files::read_file($lockfilename);
        my $waittime = $timecounter * 10;
        $waittime = installer::logger::convert_timestring($waittime);
        my $infoline = "Pool: Warning: \"$lockfilename\" blocks this process for $waittime. Lock content: \"${$filecontent}[0]\"\n";
        $installer::logger::Info->print("\n");
        $installer::logger::Info->printf("... %s", $infoline);
        $installer::logger::Lang->print("\n");
        $installer::logger::Lang->print($infoline);
    }
}

############################################################################
# This function logs some information, that can be used to find
# pool problems.
############################################################################

sub log_pool_info
{
    my ( $file_exists ) = @_;

    my $infoline = "";

    # Content saved in
    # $installer::globals::savelockfilecontent = installer::files::read_file($filename);
    # $installer::globals::savelockfilename = $filename;

    if ( $file_exists )
    {
        $installer::logger::Lang->print("\n");
        $installer::logger::Lang->printf(
            "Pool Problem: Lock file \"%s\" belongs to another process. This process has session id: %s.\n",
            $installer::globals::savelockfilename,
            $installer::globals::sessionid);
        $installer::logger::Lang->print("Content of Lock file:\n");
        foreach my $line ( @{$installer::globals::savelockfilecontent} )
        {
            $installer::logger::Lang->print($line);
        }
    }
    else
    {
        $installer::logger::Lang->print("\n");
        $installer::logger::Lang->printf(
            "Pool Problem: Lock file \"%s\" does not exist anymore (this process has session id: %s).\n",
            $installer::globals::savelockfilename,
            $installer::globals::sessionid);
    }
}

############################################################################
# Checking, if this process is the owner of the lock file in the pool.
# This can be determined by the Process ID, that is written at the
# beginning of the first line into the lock file.
############################################################################

sub process_is_owner
{
    my ( $filename ) = @_;

    my $process_is_owner = 0;

    $installer::globals::savelockfilecontent = installer::files::read_file($filename);
    $installer::globals::savelockfilename = $filename;

    if ( ${$installer::globals::savelockfilecontent}[0] =~ /^\s*\Q$installer::globals::sessionid\E\s+/ ) { $process_is_owner = 1; }

    return $process_is_owner;
}

####################################################
# Removing a package from installation set, if
# there were pooling problems.
####################################################

sub remove_package_from_installset
{
    my ($newpackagepath) = @_;

    $installer::logger::Lang->printf("Pool problem: Removing package \"%s\" from installation set!\n",
        $newpackagepath);

    if ( -f $newpackagepath ) { unlink $newpackagepath; }
    if ( -d $newpackagepath ) { installer::systemactions::remove_complete_directory($newpackagepath, 1); }

    # Keeping the content of @installer::globals::installsetcontent up to date. Removing the last package.
    pop(@installer::globals::installsetcontent);
}

####################################################
# Check, if the package is in the pool and if
# there are no changes in the package.
####################################################

sub package_is_up_to_date
{
    my ($allvariables, $onepackage, $packagename, $newepmcontent, $filesinpackage, $installdir, $subdir, $languagestringref) = @_;

    $installer::logger::Info->printf("... checking pool package ...\n", $packagename);

    installer::logger::include_header_into_logfile("Checking package in pool: $packagename");

    if ( ! $installer::globals::poolpathset ) { installer::packagepool::set_pool_path(); }
    if ( ! $installer::globals::sessionidset ) { installer::packagepool::set_sessionid(); }

    my $infoline = "";
    # Resetting some variables for this package
    my $package_is_up_to_date = 0;
    my $realpackagename = "";
    my $oldepmcontent = "";
    my $waited_for_check = 0;
    my $waited_for_lock = 0;
    $installer::globals::newpcfcontentcalculated = 0;
    %installer::globals::pcfdifflist = ();
    @installer::globals::pcfdiffcomment = ();
    @installer::globals::epmdifflist = ();

    # Reading the package content file, if this file exists (extension *.pcf)
    my $filename = $installer::globals::poolpath . $installer::globals::separator . $packagename . ".pcf";
    my $checkfilename = $installer::globals::poolpath . $installer::globals::separator . $packagename . ".pcf.check";
    my $lockfilename = $installer::globals::poolpath . $installer::globals::separator . $packagename . ".pcf.lock";
    # Saving name in global variable, so that this file can be removed somewhere else (at the end of "put_content_into_pool").
    $installer::globals::poolcheckfilename = $checkfilename;
    $installer::globals::poollockfilename = $lockfilename;

    my @checkfilecontent = ("$installer::globals::sessionid $installer::globals::product $$languagestringref $checkfilename");  # $$ is the process id
    my @lockfilecontent = ("$installer::globals::sessionid $installer::globals::product $$languagestringref $lockfilename");    # $$ is the process id

    # Waiting, step 1
    # Checking, if another process checks this package at the moment
    my $timecounter = 0;
    while ( -f $checkfilename )
    {
        $timecounter++;

        # including an exit to enable creation of other packages
        if (( $timecounter == 1 ) && ( ! exists($installer::globals::poolshiftedpackages{$packagename}) ))
        {
            $package_is_up_to_date = 3; # repeat this package later
            return $package_is_up_to_date;
        }

        $infoline = "Pool: $checkfilename exists. WAITING 10 seconds ($timecounter).\n";
        if ( $timecounter == 1 )
        {
            $installer::logger::Info->print("\n");
        }
        $installer::logger::Info->printf("... %s", $infoline);
        $installer::logger::Lang->print($infoline);
        if ( $timecounter % 100 == 0 ) { check_pool_exit($checkfilename, $timecounter); }
        sleep 10; # process sleeps 10 seconds
        $waited_for_check = 1;
    }

    # Creating file, showing that this package is checked at the moment by this process. No other process can reach this.
    installer::files::save_file($checkfilename, \@checkfilecontent);    # Creating the Lock, to check this package. This blocks all other processes.
    $installer::globals::processhaspoolcheckfile = 1;

    # Check, if the Lock file creation was really successful
    if ( ! -f $checkfilename )
    {
        $infoline = "Pool problem: Pool lock file \"$checkfilename\" could not be created successfully or was removed by another process (A)!\n";
        $installer::logger::Lang->print($infoline);
        log_pool_info(0);
        $package_is_up_to_date = 4; # repeat this package
        return $package_is_up_to_date;
    }

    if ( ! process_is_owner($checkfilename) )
    {
        $infoline = "Pool problem: Pool lock file \"$checkfilename\" belongs to another process (A)!\n";
        $installer::logger::Lang->print($infoline);
        log_pool_info(1);
        $package_is_up_to_date = 4; # repeat this package
        return $package_is_up_to_date;
    }

    $infoline = "Pool: Created file: $checkfilename\n";
    $installer::logger::Lang->print($infoline);
    if ( $waited_for_check )
    {
        $installer::logger::Info->printf("... %s", $infoline);
    }

    # Waiting, step 2
    # Checking, if another process creates this package at the moment
    $timecounter = 0;
    while ( -f $lockfilename )
    {
        $timecounter++;
        $infoline = "Pool: $lockfilename exists. WAITING 10 seconds ($timecounter).\n";
        if ( $timecounter == 1 )
        {
            $installer::logger::Info->print("\n");
        }
        $installer::logger::Info->printf("... %s", $infoline);
        $installer::logger::Lang->print($infoline);
        if ( $timecounter % 100 == 0 ) { check_pool_exit($lockfilename, $timecounter); }
        sleep 10; # process sleeps 10 seconds
        $waited_for_lock = 1;
    }

    # No lock file exists, therefore no process creates this package at the moment. Check can be done now.
    if ( $waited_for_lock )
    {
        $installer::logger::Info->printf("... Pool: Proceeding, %s was removed.\n", $lockfilename);
    }

    my $package_already_exists = 0;

    if ( -f $filename )
    {
        # Calculating content for pcf file
        $installer::globals::newpcfcontent = calculate_current_content($filesinpackage, $packagename);
        $installer::globals::newpcfcontentcalculated = 1;

        # reading the existing pcf file
        ($realpackagename, $oldpcfcontent, $oldepmcontent) = read_pcf_content($filename);

        # First check: Package has to exist in pool (directories on Solaris)
        my $fullpackage = $installer::globals::poolpath . $installer::globals::separator . $realpackagename;
        if ( $installer::globals::issolarisbuild ) { $fullpackage = $fullpackage . ".tar"; }
        if ( -f $fullpackage )
        {
            $package_already_exists = 1;
            # Second check: Only files
            my $content_is_identical = compare_package_content($oldpcfcontent, $installer::globals::newpcfcontent);

            # Third check for Unix: Changes in the epm file?
            if (( $content_is_identical ) && ( ! $installer::globals::iswindowsbuild ))
            {
                $content_is_identical = compare_epm_content($oldepmcontent, $newepmcontent);
            }

            if ( $content_is_identical ) { $package_is_up_to_date = 1; }
        }
    }

    if ( $package_is_up_to_date )
    {
        $infoline = "Pool: $packagename: No new content, using existing package\n";
        $installer::logger::Lang->print($infoline);
        $installer::logger::Info->printf("... using package from pool\n");
    }
    else
    {
        if ( $package_already_exists )
        {
            $infoline = "Pool: $packagename: Contains new content, creating new package. Differences:\n";
            $installer::logger::Lang->print($infoline);
            foreach my $dest ( sort keys %installer::globals::pcfdifflist )
            {
                $installer::logger::Lang->printf("%s\n", $dest);
            }
            foreach my $dest ( @installer::globals::pcfdiffcomment )
            {
                $installer::logger::Lang->printf("%s\n", $dest);
            }
            foreach my $dest ( @installer::globals::epmdifflist )
            {
                $installer::logger::Lang->printf("%s\n", $dest);
            }
        }
        else
        {
            $infoline = "Pool: $packagename: Does not exist in pool.\n";
            $installer::logger::Lang->print($infoline);
        }

        $installer::logger::Info->printf("... packaging required\n");
        %installer::globals::xpdpackageinfo = (); # reset the filled hash, because the package cannot be used.

        # Creating lock mechanism, so that other processes do not create this package, too.
        installer::files::save_file($lockfilename, \@lockfilecontent);      # Creating the Lock, to create this package (Lock for check still exists).
        $installer::globals::processhaspoollockfile = 1;

        # Check if creation of Lock file was really successful

        if ( ! -f $lockfilename )
        {
            $infoline = "Pool problem: Pool lock file \"$lockfilename\" could not be created successfully or was removed by another process (D)!\n";
            $installer::logger::Lang->print($infoline);
            log_pool_info(0);
            $package_is_up_to_date = 4; # repeat this package
            return $package_is_up_to_date;
        }

        if ( ! process_is_owner($lockfilename) )
        {
            $infoline = "Pool problem: Pool lock file \"$lockfilename\" belongs to another process (D)!\n";
            $installer::logger::Lang->print($infoline);
            log_pool_info(1);
            $package_is_up_to_date = 4; # repeat this package
            return $package_is_up_to_date;
        }

        $infoline = "Pool: Created file: $lockfilename\n";
        $installer::logger::Lang->print($infoline);
    }

    my $newpackagepath = "";

    if ( $package_is_up_to_date )
    {
        # Before the package is copied into the installation set, it has to be checked, if this process is really the owner of this lock file..
        # Check, if lock file still exists and if this process is the owner.

        if ( ! -f $checkfilename )
        {
            $infoline = "Pool problem: Pool lock file \"$checkfilename\" was removed by another process (B)!\n";
            $installer::logger::Lang->print($infoline);
            log_pool_info(0);
            $package_is_up_to_date = 4; # repeat this package
            return $package_is_up_to_date;
        }

        if ( ! process_is_owner($checkfilename) )
        {
            $infoline = "Pool problem: Pool lock file \"$checkfilename\" belongs to another process (B)!\n";
            $installer::logger::Lang->print($infoline);
            log_pool_info(1);
            $package_is_up_to_date = 4; # repeat this package
            return $package_is_up_to_date;
        }

        # Copying the package from the pool into the installation set
        $newpackagepath = copy_package_from_pool($installdir, $subdir, $realpackagename);
    }

    # Before the lock file in the pool can be removed, it has to be checked, if this process is still the owner of this lock file.
    # Check, if lock file still exists and if this process is the owner.
    if ( ! -f $checkfilename )
    {
        $infoline = "Pool problem: Pool lock file \"$checkfilename\" was removed by another process (C)!\n";
        $installer::logger::Lang->print($infoline);
        log_pool_info(0);

        # removing new package from installation set
        if ( $newpackagepath ne "" ) { remove_package_from_installset($newpackagepath); }   # A file was copied and a problem occured with pooling

        $package_is_up_to_date = 4; # repeat this package
        return $package_is_up_to_date;
    }

    if ( ! process_is_owner($checkfilename) )
    {
        $infoline = "Pool problem: Pool lock file \"$checkfilename\" belongs to another process (C)!\n";
        $installer::logger::Lang->print($infoline);
        log_pool_info(1);

        # removing new package from installation set
        if ( $newpackagepath ne "" ) { remove_package_from_installset($newpackagepath); }   # A file was copied and a problem occured with pooling

        $package_is_up_to_date = 4; # repeat this package
        return $package_is_up_to_date;
    }

    # Removing the check file, releasing this package for the next process.
    # The Lock to create this package still exists, if required.
    unlink $checkfilename;
    $installer::globals::processhaspoolcheckfile = 0;
    $infoline = "Pool: Removing file: $checkfilename\n";
    $installer::logger::Lang->print($infoline);

    # Last chance before packaging starts, to check, if this process is really still owner
    # of the packaging lock file. If not, this packaging process can be repeated.
    if ( $installer::globals::processhaspoollockfile )
    {
        if ( ! -f $lockfilename )
        {
            $infoline = "Pool problem: Pool lock file \"$lockfilename\" was removed by another process (E)!\n";
            $installer::logger::Lang->print($infoline);
            log_pool_info(0);
            $package_is_up_to_date = 4; # repeat this package
            return $package_is_up_to_date;
        }

        if ( ! process_is_owner($lockfilename) )
        {
            $infoline = "Pool problem: Pool lock file \"$lockfilename\" belongs to another process (E)!\n";
            $installer::logger::Lang->print($infoline);
            log_pool_info(1);
            $package_is_up_to_date = 4; # repeat this package
            return $package_is_up_to_date;
        }
    }

    # Collecting log information
    if ( $package_is_up_to_date == 1 ) { $installer::globals::poolpackages{$packagename} = 1; }
    if ( $package_is_up_to_date == 0 )
    {
        my @packreasons = ();
        if ( $package_already_exists )
        {
            $infoline = "\t\tPool: $packagename: Contains new content, creating new package. Differences:\n";
            push( @packreasons, $infoline);
            foreach my $dest ( sort keys %installer::globals::pcfdifflist ) { push( @packreasons, "\t\t$dest\n"); }
            foreach my $dest ( @installer::globals::pcfdiffcomment ) { push( @packreasons, "\t\t$dest"); }
            foreach my $dest ( @installer::globals::epmdifflist ) { push( @packreasons, "\t\t$dest"); }
        }
        else
        {
            $infoline = "\t\tPool: $packagename: Does not exist in pool.\n";
            push( @packreasons, $infoline);
        }

        $installer::globals::createpackages{$packagename} = \@packreasons;
    }

    return $package_is_up_to_date;
}

###################################################
# Determine, which package was created newly
###################################################

sub determine_new_packagename
{
    my ( $dir ) = @_;

    my ($newcontent, $allcontent) = installer::systemactions::find_new_content_in_directory($dir, \@installer::globals::installsetcontent);
    @installer::globals::installsetcontent = ();
    foreach my $element ( @{$allcontent} ) { push(@installer::globals::installsetcontent, $element); }

    my $newentriesnumber = $#{$newcontent} + 1;
    if ( $newentriesnumber > 1 )
    {
        my $newpackages = "";
        foreach my $onepackage ( @{$newcontent} ) { $newpackages = $newpackages . " " . $onepackage; }
        installer::exiter::exit_program("ERROR: More than one new package in directory $dir ($newpackages)", "determine_new_packagename (packagepool)");
    }
    elsif ( $newentriesnumber < 1 )
    {
        installer::exiter::exit_program("ERROR: No new package in directory $dir", "determine_new_packagename (packagepool)");
    }
    my $newpackage = ${$newcontent}[0];

    return $newpackage;
}

####################################################
# Including content into the package pool
####################################################

sub put_content_into_pool
{
    my ($packagename, $installdir, $subdir, $filesinpackage, $epmfilecontent) = @_;

    my $infoline = "";

    my $fullinstalldir = $installdir . $installer::globals::separator . $subdir;
    my $fullrealpackagename = determine_new_packagename($fullinstalldir);
    my $realpackagename = $fullrealpackagename;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$realpackagename);

    installer::logger::include_header_into_logfile("Adding content into the package pool: $realpackagename (PackageName: $packagename)");

    # Calculating content for pcf file, if not already done in "package_is_up_to_date"
    if ( ! $installer::globals::newpcfcontentcalculated )
    {
        $installer::globals::newpcfcontent = calculate_current_content($filesinpackage, $packagename);
        $installer::globals::newpcfcontentcalculated = 1;
    }

    # Determining md5sum and FileSize for the new package and saving in pcf file
    my $md5sum = installer::xpdinstaller::get_md5_value($fullrealpackagename);
    my $filesize = installer::xpdinstaller::get_size_value($fullrealpackagename);
    my $fullpackagename = installer::xpdinstaller::get_fullpkgname_value($fullrealpackagename);
    my $pkgversion = installer::xpdinstaller::get_pkgversion_value($fullrealpackagename);

    # Put package content file (pcf) into pool
    my $pcffilename = $installer::globals::poolpath . $installer::globals::separator . $packagename . ".pcf";
    create_pcfcontent_file($realpackagename, $md5sum, $filesize, $fullpackagename, $pkgversion, $epmfilecontent, $pcffilename);

    # Creating xpd info
    $installer::globals::xpdpackageinfo{'FileSize'} = $filesize;
    $installer::globals::xpdpackageinfo{'FullPackageName'} = $fullpackagename;
    $installer::globals::xpdpackageinfo{'md5sum'} = $md5sum;
    $installer::globals::xpdpackageinfo{'RealPackageName'} = $realpackagename;
    $installer::globals::xpdpackageinfo{'PkgVersion'} = $pkgversion;

    # Put package into pool
    $infoline = "Pool: Adding package \"$packagename\" into pool.\n";
    $installer::logger::Lang->print($infoline);

    # Copying with unique name, containing PID. Only renaming if everything was fine.
    my $realdestination = "";
    my $uniquedestination = "";
    if ( -f $fullrealpackagename )
    {
        $realdestination = $installer::globals::poolpath . $installer::globals::separator . $realpackagename;
        $uniquedestination = $realdestination . "." . $installer::globals::sessionid;
        installer::systemactions::copy_one_file($fullrealpackagename, $uniquedestination);
    }

    # Copying Solaris packages (as tar files)
    if ( -d $fullrealpackagename )
    {
        my $tarfilename = $packagename . ".tar";
        my $fulltarfilename = $fullinstalldir . $installer::globals::separator . $tarfilename;
        my $size = installer::worker::tar_package($fullinstalldir, $packagename, $tarfilename, $installer::globals::getuidpath);
        if (( ! -f $fulltarfilename ) || ( ! ( $size > 0 ))) { installer::exiter::exit_program("ERROR: Missing file: $fulltarfilename", "put_content_into_pool"); }
        $realdestination = $installer::globals::poolpath . $installer::globals::separator . $tarfilename;
        $uniquedestination = $realdestination . "." . $installer::globals::sessionid;
        installer::systemactions::copy_one_file($fulltarfilename, $uniquedestination);
        unlink $fulltarfilename;
    }

    # Before the new package is renamed in the pool, it has to be checked, if this process still has the lock for this package.
    # Check, if lock file still exists and if this process is the owner. Otherwise a pool error occured.
    if ( ! -f $installer::globals::poollockfilename )
    {
        unlink $uniquedestination;  # removing file from pool
        log_pool_info(0);
        installer::exiter::exit_program("ERROR: Pool lock file \"$installer::globals::poollockfilename\" was removed by another process (F)!", "put_content_into_pool");
    }

    if ( ! process_is_owner($installer::globals::poollockfilename) )
    {
        unlink $uniquedestination;  # removing file from pool
        log_pool_info(1);
        installer::exiter::exit_program("ERROR: Pool lock file \"$installer::globals::poollockfilename\" belongs to another process (F)!", "put_content_into_pool");
    }

    # Renaming the file in the pool (atomic step)
    rename($uniquedestination, $realdestination);

    $infoline = "Pool: Renamed file: \"$uniquedestination\" to \"$realdestination\".\n";
    $installer::logger::Lang->print($infoline);

    # Before the lock file in the pool can be removed, it has to be checked, if this process is still the owner of this lock file.
    # Check, if lock file still exists and if this process is the owner. Otherwise a pool error occured.
    if ( ! -f $installer::globals::poollockfilename )
    {
        log_pool_info(0);
        installer::exiter::exit_program("ERROR: Pool lock file \"$installer::globals::poollockfilename\" was removed by another process (G)!", "put_content_into_pool");
    }

    if ( ! process_is_owner($installer::globals::poollockfilename) )
    {
        log_pool_info(1);
        installer::exiter::exit_program("ERROR: Pool lock file \"$installer::globals::poollockfilename\" belongs to another process (G)!", "put_content_into_pool");
    }

    # Removing lock file, so that other processes can use this package now
    unlink $installer::globals::poollockfilename;
    $installer::globals::processhaspoollockfile = 0;
    $infoline = "Pool: Removing file: $installer::globals::poollockfilename\n";
    $installer::logger::Lang->print($infoline);
}

###################################################################
# Copying a package from the pool into the installation set
###################################################################

sub copy_package_from_pool
{
    my ($installdir, $subdir, $packagename) = @_;

    my $infoline = "Pool: Using package \"$packagename\" from pool.\n";
    $installer::logger::Lang->print($infoline);
    my $sourcefile = $installer::globals::poolpath . $installer::globals::separator . $packagename;
    if ( $installer::globals::issolarisbuild ) { $sourcefile = $sourcefile . ".tar"; }
    if ( ! -f $sourcefile ) { installer::exiter::exit_program("ERROR: Missing package in package pool: \"$sourcefile\"", "copy_package_from_pool"); }
    my $destination = $installdir . $installer::globals::separator . $subdir;
    if ( ! -d $destination ) { installer::systemactions::create_directory($destination); }
    my $destinationfile = $destination . $installer::globals::separator . $packagename;
    if ( $installer::globals::issolarisbuild ) { $destinationfile = $destinationfile . ".tar"; }
    if ( -f $sourcefile ) { installer::systemactions::copy_one_file($sourcefile, $destinationfile); }
    # Unpacking for Solaris
    if ( $installer::globals::issolarisbuild )
    {
        my $tarfilename = $packagename . ".tar";
        installer::worker::untar_package($destination, $tarfilename, $installer::globals::getuidpath);
        unlink $destinationfile;
        $destinationfile =~ s/.tar\s*$//;
    }

    # Keeping the content of @installer::globals::installsetcontent up to date (with full pathes):
    push(@installer::globals::installsetcontent, $destinationfile);

    return $destinationfile;
}

###################################################################
# Counting keys in hash
###################################################################

sub get_count
{
    my ( $hashref ) = @_;

    my $counter = 0;
    foreach my $onekey ( keys %{$hashref} ) { $counter++; }
    return $counter;
}

###################################################################
# Logging some pool information
###################################################################

sub log_pool_statistics
{
    my $infoline = "";

    installer::logger::include_header_into_logfile("Pool statistics:");

    # Info collected in global hashes
    # %installer::globals::createpackages
    # %installer::globals::poolpackages

    my $pool_packages = get_count(\%installer::globals::poolpackages);
    my $created_packages = get_count(\%installer::globals::createpackages);

    $infoline = "Number of packages from pool: $pool_packages\n";
    $installer::logger::Lang->print($infoline);

    foreach my $packagename ( sort keys(%installer::globals::poolpackages) )
    {
        $infoline = "\t$packagename\n";
        $installer::logger::Lang->print($infoline);
    }

    $installer::logger::Lang->print("\n");
    $installer::logger::Lang->print("Number of packages that were created: %s\n", $created_packages);

    foreach my $packagename ( sort keys(%installer::globals::createpackages) )
    {
        $infoline = "\t$packagename\n";
        $installer::logger::Lang->print($infoline);
        my $reason = $installer::globals::createpackages{$packagename};

        foreach my $line (@reason)
        {
            $installer::logger::Lang->print($line);
        }
    }
}

1;
