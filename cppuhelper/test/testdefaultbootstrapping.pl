:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

#
# deliver.pl - copy from module output tree to solver
#

my $progname = "testdefaultbootstrapping";
my $defExeExt;

if ($ENV{OS} eq "WNT") {
    %services = (
                 'com.sun.star.uno.NamingService'               => 'namingservice.uno.dll',
                 'com.sun.star.reflection.CoreReflection'       => 'reflection.uno.dll',
                 'com.sun.star.script.InvocationAdapterFactory' => 'invocadapt.uno.dll',
                 );

    $defExeExt = ".exe";

}
else {
    %services = (
                 'com.sun.star.uno.NamingService'               => 'namingservice.uno.so',
                 'com.sun.star.reflection.CoreReflection'       => 'reflection.uno.so',
                 'com.sun.star.script.InvocationAdapterFactory' => 'invocadapt.uno.so'
                 );

    $defExeExt = "";
}

sub extendProgName($) {
    my $_extension = shift;
    my $_result;

    if ($ENV{OS} eq "WNT") {
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

#   system 'echo', "regcomp -register -r " . $rdb_name . " -c $service_lib";
    my $rc = system "regcomp -register -r " . $rdb_name . " -c $service_lib";


    return ! ( $rc >> 8 );
}



my $state = 1;
my @allservices;
my $allservices_rdbs="";
my $rc;
my $comment;
my $testexe;


sub registerServices() {
    use Cwd;

    # ensure that services can not be instantiated
    foreach $service ( keys %services ) {
        # ensure that the current service is not reachable
        unregisterService($service);
        $rc = !testForServices([$service], "", $testexe);
        if(!$rc) {
            $comment = $comment . "\tcouldn't unregister service " . $service . "\n";
            $state = 0;
        }


        # register the service and ensure that it is reachable
        $rc = registerService($services{$service}, $service . '.rdb');
        if(!$rc) {
            $comment = $comment . "\tcouldn't register service " . $service . "\n";
            $state = 0;
        }

        $rc = testForServices([$service], "-env:UNO_SERVICES=" . $service . ".rdb", $testexe);
        if(!$rc) {
            $comment = $comment . "\tcouldn't reach service " . $service . "\n";
            $state = 0;
        }

        # memorize all services
        if (length($allservices_rdbs)) {
            $allservices_rdbs = $allservices_rdbs . " ";
        }
        $allservices_rdbs = $allservices_rdbs . "file://" . getcwd() . "/" . $service . ".rdb";
        push @allservices, $service;
    }
}

sub testIndirection() {
    #test indirection
    $rc = testForServices(['com.sun.star.uno.NamingService'], '-env:UNO_SERVICES=${testrc:Tests:TestKey1}', $testexe);
    if (!$rc) {
        $comment = $comment . "\tindirection test not passed\n";
        $state = 0;
    }
}


sub testBeneathExe() {
    my $service = 'com.sun.star.reflection.CoreReflection';
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
            $comment = $comment . "\tbeneath executable test not passed: " . $_testexe . "\n";
            $state = 0;
        }
    }
}

sub testBeneathLib_rdb() {
    my $_service = 'com.sun.star.uno.NamingService';

    use UNO;

    my $_rdb_name;

    if ($ENV{OS} eq "WNT") {
        $_rdb_name = "UNO" . "_services.rdb";
    }
    else {
        $_rdb_name = "../lib/UNO" . "_services.rdb";
    }

    registerService($services{$_service}, $_rdb_name);
    my $_rc = UNO::tryService($_service);
    if (!$_rc) {
        $comment = $comment . "\tbeneath lib test not passed\n";
        $state = 0;
    }
    unlink $_rdb_name;
}

sub testBeneathLib_rc() {
    my $_service = 'com.sun.star.uno.NamingService';

    use UNO;

    my $_rc_name;

    if ($ENV{OS} eq "WNT") {
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
        $comment = $comment . "\tbeneath lib rc test not passed\n";
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
        $comment = $comment . "\tmulti rdb test not passed\n";
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



