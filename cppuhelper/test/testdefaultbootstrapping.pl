:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: testdefaultbootstrapping.pl,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kr $ $Date: 2001-09-03 14:28:17 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

#
# deliver.pl - copy from module output tree to solver
#

my $progname = "testdefaultbootstrapping";
my $defExeExt;

if ($ENV{GUI} eq "WNT") {
    %services = (
                 'com.sun.star.uno.NamingService' => 'namingservice.dll',
                 'com.sun.star.script.Invocation' => 'inv.dll',
                 'com.sun.star.script.Converter' => 'tcv.dll'
                 );

    $defExeExt = ".exe";

}
else {
    %services = (
                 'com.sun.star.uno.NamingService' => 'libnamingservice.so',
                 'com.sun.star.script.Invocation' => 'libinv.so',
                 'com.sun.star.script.Converter' => 'libtcv.so'
                 );

    $defExeExt = "";
}

sub extendProgName($) {
    my $_extension = shift;
    my $_result;

    if ($ENV{GUI} eq "WNT") {
        $_result = $progname . $_extension;
    }
    else {
        $_result = $ENV{PWD} . "/" . $progname . $_extension;
    }

    return $_result;
}


sub rmDefRDB() {
    unlink $progname . "_services.rdb";
}


sub unregisterService($){
    my $service_name = shift;

    my $rdb_name = $service_name . '.rdb';

    unlink $rdb_name;

    return 1;
}

sub testForServices($$$) {
    my $_services = shift;
    my $_pars     = shift;
    my $_testexe  = shift;


#   my $_rc = system 'echo', $_testexe, @{$_services}, $_pars;
    my $_rc = system $_testexe, @{$_services}, $_pars;

    return $_rc >> 8;
}


sub registerService($$){
    my $service_lib  = shift;
    my $rdb_name = shift;

    system 'echo', "regcomp -register -r " . $rdb_name . " -c $service_lib";
    my $rc = system "regcomp -register -r " . $rdb_name . " -c $service_lib";

    return ! ( $rc >> 8 );
}



my $state = 1;
my @allservices;
my $allservices_rdbs;
my $rc;
my $comment;
my $testexe;


sub registerServices() {
    # ensure that services can not be instantiated
    foreach $service ( keys %services ) {
        # ensure that the current service is not reachable
        unregisterService($service);
        $rc = !testForServices([$service], "", $testexe);
        if(!$rc) {
            $comment = $comment . "couldn't unregister service " . $service . "\n";
            $state = 0;
        }


        # register the service and ensure that it is reachable
        $rc = registerService($services{$service}, $service . '.rdb');
        if(!$rc) {
            $comment = $comment . "couldn't register service " . $service . "\n";
            $state = 0;
        }

        $rc = testForServices([$service], "-env:UNO_SERVICES=" . $service . ".rdb", $testexe);
        if(!$rc) {
            $comment = $comment . "couldn't reeach service " . $service . "\n";
            $state = 0;
        }

        # memorize all services
        $allservices_rdbs = $allservices_rdbs . $service . ".rdb" . " ";
        push @allservices, $service;
    }
}

sub testIndirection() {
    #test indirection
    $rc = testForServices(['com.sun.star.script.Invocation'], '-env:UNO_SERVICES=${testrc:Tests:TestKey1}', $testexe);
    if (!$rc) {
        $comment = $comment . "indirection test not passed\n";
        $state = 0;
    }
}


sub testBeneathExe() {
    my $service = 'com.sun.star.script.Invocation';
    my $_testexe;

    my @_exes = (extendProgName(".exe"),
                 extendProgName(".Exe"),
                 extendProgName(".bin"),
                 extendProgName(".Bin"));

    foreach $_testexe ( @_exes ) {
        #test rdb found beneath executable
        registerService($services{$service}, $progname . "_services.rdb");
        my $_rc = testForServices([$service], "", $_testexe);
        if (!$_rc) {
            $comment = $comment . "beneath executable test not passed: " . $_testexe . "\n";
            $state = 0;
        }
    }
}

sub testBeneathLib_rdb() {
    my $_service = 'com.sun.star.script.Invocation';

    use UNO;

    my $_rdb_name;

    if ($ENV{GUI} eq "WNT") {
        $_rdb_name = "UNO" . "_services.rdb";
    }
    else {
        $_rdb_name = "../lib/UNO" . "_services.rdb";
    }

    registerService($services{$_service}, $_rdb_name);
    my $_rc = UNO::tryService($_service);
    if (!$_rc) {
        $comment = $comment. "beneath lib test not passed\n";
        $state = 0;
    }
    unlink $_rdb_name;
}

sub testBeneathLib_rc() {
    my $_service = 'com.sun.star.script.Invocation';

    use UNO;

    my $_rc_name;

    if ($ENV{GUI} eq "WNT") {
        $_rc_name = "UNO.ini";
    }
    else {
        $_rc_name = "../lib/UNOrc";
    }

    my $_rdb_name = "../lib/test.rdb";

    my $_handle;
    open $_handle, ">" . $_rc_name;
    print $_handle "UNO_SERVICES=" . $_rdb_name . "\n";
    close $_handle;

    registerService($services{$_service}, $_rdb_name);
    my $_rc = UNO::tryService($_service);
    if (!$_rc) {
        $comment = $comment. "beneath lib rc test not passed\n";
        $state = 0;
    }
    unlink $_rdb_name;
    unlink $_rc_name;
}

sub testAllAvailable() {
    # test that all services are reachable through different rdbs
    # change the directory to ensure, that all paths become expanded
    chdir "..";

    $rc = testForServices(\@allservices, "-env:UNO_SERVICES=" . $allservices_rdbs, $testexe);
    if (!$rc) {
        $comment = $comment. "multi rdb test not passed\n";
        $state = 0;
    }
}

$testexe = extendProgName($defExeExt);

rmDefRDB();
registerServices();
#print "alls:", @allservices, $allservices_rdbs, "\n";

testIndirection();
testBeneathExe();
testBeneathLib_rc();
testBeneathLib_rdb();
testAllAvailable();

print "**************************\n";
if($state) {
    print "****** tests passed ******\n";
}
else {
    print "**** tests NOT passed ****\n";
    print "Commnent:\n", $comment, "\n";
}
print "**************************\n";



