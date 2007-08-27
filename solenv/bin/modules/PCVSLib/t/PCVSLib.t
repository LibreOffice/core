#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: PCVSLib.t,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2007-08-27 13:36:21 $
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

# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl PCVSLib.t'

#########################

# change 'tests => 2' to 'tests => last_test_to_print';

use Test;
BEGIN { plan tests => 70 };
use IO::File;
use Cwd;
use File::Compare;
use File::Copy;
use File::Temp qw(tempdir);
use PCVSLib::Root;
use PCVSLib::Entry;
use PCVSLib::Credentials;
use PCVSLib::LogHandle;
use PCVSLib::Client;
use PCVSLib::Command;
use PCVSLib::EventHandler;
use PCVSLib::Directory;
use PCVSLib::Listener;
use PCVSLib;
ok(1); # If we made it this far, we're ok.

#########################

# chdir into test directory if started via 'make test'
if ( -d 't' ) {
    chdir('t') or die "can't chdir into directory 't'";
}

#### setup the test repository
my $test_repository = tempdir(CLEANUP=>1);
system("cvs -d $test_repository init");
unlink("$test_repository/CVSROOT/config") or die "can't unlink $test_repository/CVSROOT/config: $!" ;

my $import_dir = tempdir(CLEANUP=>1);
open(FILE_AAA, ">$import_dir/aaa")  or die "can't open $import_dir/aaa: $!";
print FILE_AAA "aaa\n";
close(FILE_AAA);
open(FILE_BBB, ">$import_dir/bbb")  or die "can't open $import_dir/bbb: $!";
print FILE_BBB "bbb\n";
close(FILE_BBB);
open(FILE_CCC, ">$import_dir/ccc")  or die "can't open $import_dir/ccc $!";
print FILE_CCC "ccc\n";
print FILE_CCC "second line\n";
print FILE_CCC "third line\n";
print FILE_CCC "fourth line\n";
close(FILE_CCC);
chmod(0775,"$import_dir/ccc");

open(FILE_HUGE, ">$import_dir/huge")  or die "can't open $import_dir/huge $!";
my @files = glob("../lib/PCVSLib/*.pm");
foreach (@files) {
    open(FILE, "<$_") or die "can't open $import_dir/$_ $!";
    while(<FILE>) {
        print FILE_HUGE $_;
    }
    close(FILE);
}
close(FILE_HUGE);

open(FILE_BINARY, ">$import_dir/binary.bin")  or die "can't open $import_dir/binary.bin $!";
for (my $i=0; $i<8192; $i++) {
    push(@random, rand());
}
my $binary_string = pack("d*", @random);
syswrite(FILE_BINARY, $binary_string, 8192*8);
close(FILE_BINARY);

mkdir("$import_dir/subdir1");
open(FILE_AAA, ">$import_dir/subdir1/aaa")  or die "can't open $import_dir/subdir1/aaa: $!";
print FILE_AAA "aaa\n";
close(FILE_AAA);
open(FILE_BBB, ">$import_dir/subdir1/bbb")  or die "can't open $import_dir/subdir1/bbb: $!";
print FILE_BBB "bbb\n";
close(FILE_BBB);
open(FILE_CCC, ">$import_dir/subdir1/ccc")  or die "can't open $import_dir/subdir1/ccc $!";
print FILE_CCC "ccc\n";
close(FILE_CCC);

mkdir("$import_dir/subdir2");
open(FILE_XYZ, ">$import_dir/subdir2/xyz")  or die "can't open $import_dir/subdir2/xyz: $!";
print FILE_XYZ "xyz\n";
close(FILE_XYZ);
mkdir("$import_dir/subdir3");


my $save_dir = getcwd();
chdir($import_dir);
system("cvs -d :local:$test_repository import -W \"*.bin -k 'b'\" -m'initial import' testmodule demo_vendortag DEMO_RELEASE_TAG 2>&1 > /dev/null");
chdir($save_dir);

open(CVSCONFIG, ">$test_repository/CVSROOT/config") or die "can't open $test_repository/CVSROOT/config: $!" ;
print CVSCONFIG "SystemAuth=no\n";
close(CVSCONFIG);
open(CVSPASS, ">$test_repository/CVSROOT/passwd")  or die "can't open $test_repository/CVSROOT/passwd: $!";
my $login = getlogin || getpwuid($<);
print CVSPASS "user:wUkrxyBVZk0V2:$login\n";
close(CVSPASS);

#### setup the test workspace
my $test_workspace = tempdir(CLEANUP=>1);
open(CVSPASS, ">$test_workspace/.cvspass");
print CVSPASS ':pserver:user@localhost:' . '8888' . $test_repository . ' Ay=er=03tr3cy,i' . "\n";
close(CVSPASS);

#### tests which don't need the daemon
print "# Testing PCVSLib::Root\n";
my $root_string = ':pserver:some_user@cvsserver.somewhere.org:/path_to_repository';
my $root1 = PCVSLib::Root->new($root_string);
ok($root1->method() eq 'pserver' 
    && $root1->user() eq 'some_user'
    && $root1->host() eq 'cvsserver.somewhere.org'
    && $root1->port() eq 2401
    && $root1->root_dir() eq '/path_to_repository', 1);
ok($root1->to_string() eq $root_string, 1);

$root_string = ':pserver:some_user@cvsserver.somewhere.org:4711/path_to_repository';
my $root2 = PCVSLib::Root->new($root_string);
ok($root2->method() eq 'pserver' 
    && $root2->user() eq 'some_user'
    && $root2->host() eq 'cvsserver.somewhere.org'
    && $root2->port() eq 4711
    && $root2->root_dir() eq '/path_to_repository', 1);
ok($root2->to_string_with_port() eq $root_string, 1);

print "# Testing PCVSLib::Time\n";
my $time = time();
my $time_str = PCVSLib::Time::seconds_to_rfc822_1123($time);
ok(PCVSLib::Time::rfc822_1123_to_seconds($time_str) == $time, 1);
$time_str = PCVSLib::Time::seconds_to_timestr($time);
ok(PCVSLib::Time::timestr_to_seconds($time_str) == $time, 1);
$time_str = "13 Jun 2003 14:37:05 +0200";
ok(PCVSLib::Time::rfc822_1123_to_seconds($time_str) == 1055507825, 1);

print "# Testing PCVSLib::Entry\n";
my $entry1 = PCVSLib::Entry->new('/test/0/dummy timestamp//');
my $entry2 = PCVSLib::Entry->new('/test1.cpp/1.21/Fri Jun 13 12:37:05 2003//');
my $entry3 = PCVSLib::Entry->new('/test2.cpp/1.14/Result of merge+Wed Jan 19 14:52:04 2005//');
my $entry4 = PCVSLib::Entry->new('/test3.cpp/1.14/Result of merge//');
ok($entry1->name() eq 'test');
ok($entry2->timestamp() == 1055507825);
ok($entry3->is_conflict());
ok($entry4->is_merge() && !$entry4->is_conflict());

print "# Testing PCVSLib::Credentials\n";
my $credentials_fail = PCVSLib::Credentials->new();
eval { my $password_fail = $credentials_fail->get_password($root1) };
ok($@ =~ /PCVSLIB::Credentials::get_password\(\): missing entry in/);


print "# Testing PCVSLib::LogHandle\n";
my $fh1 = IO::File->new_tmpfile();
my $fh2 = IO::File->new_tmpfile();

my $loggedhandle = PCVSLib::LogHandle->new($fh1, $fh2);
ok(1);

$loggedhandle->print("just a small text\n");
$fh1->flush();
$fh2->flush();
ok(1);

$fh1->seek(0, SEEK_SET);
$loggedhandle->getline();

$fh2->seek(0, SEEK_SET);
# logged print
my $cont = $fh2->getline();
ok($cont eq "C: just a small text\n", 1);

# logged read
$cont = $fh2->getline();
ok($cont eq "S: just a small text\n", 1);

$fh1->close();
$fh2->close();

print "# Testing PCVSLib::EventHandler\n";

my $test_listener1 = TestListener->new();
my $test_listener2 = TestListener->new();
my $test_listener3 = TestListener->new();

my $test_event_handler = PCVSLib::EventHandler->new();
$test_event_handler->add_listener($test_listener1);
$test_event_handler->add_listener($test_listener2);
$test_event_handler->add_listener($test_listener3);

eval { $test_event_handler->add_listener("This ain't no listener"); };
ok($@ =~ /PCVSLIB::EventHandler::add_listener\(\): added object is not a listener/);

my $event = PCVSLib::MessageEvent->new("TestEvent");
$test_event_handler->send_event($event);
ok($test_listener3->count(), 1);
$test_event_handler->remove_listener($test_listener2);
$test_event_handler->send_event($event);
ok($test_listener2->count(), 1);
ok($test_listener3->count(), 2);

#### start the daemon
my $daemon_pid;
if ( !($daemon_pid = fork()) ) {
    # child
    exec('perl', 'daemon.pl', $test_repository, '8888')
}

sleep(1);

print "# Testing PCVSLib::Connection\n";
my $connection_root = PCVSLib::Root->new(':pserver:user@localhost:' . '8888' . $test_repository);

my $connection_fail = PCVSLib::Connection->new($connection_root, 'Anone');
eval { my $cvs_handle_fail = eval $connection_fail->open() };
ok($@ =~ /PCVSLib::Connection::pserver_open\(\): '[\w\.\-]+': authentication failed/);

my $credentials = PCVSLib::Credentials->new("$test_workspace/.cvspass");
my $password = $credentials->get_password($connection_root);
my $connection_ok = PCVSLib::Connection->new($connection_root, $password);

my $cvs_handle_ok = $connection_ok->open();
ok(1);
$connection_ok->close();
ok(1);

print "# Testing PCVSLib::Client\n";

my $log_dir = cwd();

my $connection = PCVSLib::Connection->new($connection_root, $password);
my $log_handle = IO::File->new(">$log_dir/log");
my $io_handle = $connection->open();
$connection->io_handle(PCVSLib::LogHandle->new($io_handle, $log_handle));

my $client = PCVSLib::Client->new($connection);

my $event_handler = PCVSLib::EventHandler->new();
my $listener = TestListener->new();
$event_handler->add_listener($listener);

my $command = PCVSLib::VersionCommand->new($event_handler);
$client->execute_command($command);
ok($listener->is_success(), 1);
ok($listener->get_message() =~ /^Concurrent Versions System/, 1);
print "# ". $listener->get_message() . "\n";

$command = PCVSLib::ValidRequestsCommand->new($event_handler);
$client->execute_command($command);
ok($listener->is_success(), 1);
ok(@{$listener->get_valid_requests()} > 10, 1);
print "# ". join(" ", @{$listener->get_valid_requests()}) . "\n";

print "# Testing rlog support\n";
my $rlog_listener = LogListener->new();
$event_handler->add_listener($rlog_listener);
my $rlog_command = PCVSLib::RLogCommand->new($event_handler);
$rlog_command->file_list(['testmodule']);
$client->execute_command($rlog_command);
ok($rlog_listener->is_success(), 1);
foreach ( @{$rlog_listener->get_log()} ) {
    print "# $_\n";
}

my $rlog2_listener = LogListener->new();
$event_handler->add_listener($rlog2_listener);
my $rlog2_command = PCVSLib::RLogCommand->new($event_handler);
$rlog2_command->file_list(['testmodule/aaa']);
$client->execute_command($rlog2_command);
ok($rlog2_listener->is_success(), 1);
foreach ( @{$rlog2_listener->get_log()} ) {
    print "# $_\n";
}
$event_handler->remove_listener($rlog2_listener);
$connection->io_handle($io_handle);
$connection->close();
$log_handle->close();


# For some reasons deep in the cvs server we need a new connection after an rlog before we try
# a another log command
$connection_root = PCVSLib::Root->new(':pserver:user@localhost:' . '8888' . $test_repository);
$connection = PCVSLib::Connection->new($connection_root, $password);
$log_handle = IO::File->new(">>$log_dir/log");
$io_handle = $connection->open();
$connection->io_handle(PCVSLib::LogHandle->new($io_handle, $log_handle));
$client = PCVSLib::Client->new($connection);
print "# Testing checkout support\n";
$save_dir = getcwd();
chdir($test_workspace);
my $checkout_command = PCVSLib::CheckoutCommand->new($event_handler);
$checkout_command->file_list(['testmodule']);
$client->execute_command($checkout_command);

print "# Testing binary checkout support\n";
ok(compare("$import_dir/binary.bin", "$test_workspace/testmodule/binary.bin"), 0);

print "# Testing file permissions\n";
ok (-x "$test_workspace/testmodule/ccc", 1);

print "# Testing PCVSLib::Directory\n";

my $directory = PCVSLib::Directory->new("$test_workspace/testmodule");
my $directory_rep = $directory->repository();
if ( $directory_rep =~ /^\// ) {
    ok($directory_rep eq ($directory->root()->root_dir() . "/testmodule"), 1);
}
else {
    ok($directory_rep eq 'testmodule', 1);
}
my $directory_root = $directory->root()->to_string_with_port();
ok($directory_root eq ":pserver:user\@localhost:8888$test_repository", 1);

my $entry = $directory->get_entry('bbb');
print "# " . $entry->to_string() . "\n";
ok($entry->name() eq 'bbb', 1);
ok ($directory->is_modified('bbb') == 0, 1);
utime undef, undef, "$test_workspace/testmodule/bbb";
ok ($directory->is_modified('bbb') == 1, 1);
ok ($directory->is_questionable('bbb') == 0, 1);
system("echo ttt > $test_workspace/testmodule/ttt");
ok ($directory->is_questionable('ttt') == 1, 1);
ok ($directory->get_entry('binary.bin')->is_binary() == 1, 1);

$event_handler->remove_listener($listener);

chdir("$test_workspace/testmodule");

print "# Testing PCVSLib::LogCommand\n";
my $log_listener = LogListener->new();
$event_handler->add_listener($log_listener);
my $log_command = PCVSLib::LogCommand->new($event_handler);
$log_command->file_list(['ccc', 'bbb', 'aaa', 'subdir1/bbb', 'subdir1/ccc', 'subdir2/xyz', 'subdir1/aaa']);
$client->execute_command($log_command);
ok($log_listener->is_success(), 1);
foreach ( @{$log_listener->get_log()} ) {
    print "# $_\n";
}
$event_handler->remove_listener($log_listener);

print "# Testing PCVSLib::TagCommand\n";
my $tag_listener = LogListener->new();
$event_handler->add_listener($tag_listener);
my $tag_command = PCVSLib::TagCommand->new($event_handler);
$tag_command->tag('test_tag1');
$tag_command->file_list(['ccc']);
$client->execute_command($tag_command);
ok($tag_listener->is_success(), 1);
foreach ( @{$tag_listener->get_log()} ) {
    print "# $_\n";
}
$event_handler->remove_listener($tag_listener);

$tag_listener = LogListener->new();
$event_handler->add_listener($tag_listener);
$tag_command = PCVSLib::TagCommand->new($event_handler);
$tag_command->branch('branch_tag1');
$tag_command->file_list(['aaa']);
$client->execute_command($tag_command);
ok($tag_listener->is_success(), 1);
foreach ( @{$tag_listener->get_log()} ) {
    print "# $_\n";
}
$event_handler->remove_listener($tag_listener);

$log_listener = LogListener->new();
$event_handler->add_listener($log_listener);
$log_command = PCVSLib::LogCommand->new($event_handler);
$log_command->file_list(['aaa', 'ccc']);
$client->execute_command($log_command);
ok($log_listener->is_success(), 1);
ok(grep(/test_tag1/, @{$log_listener->get_log()}), 1);
ok(grep(/branch_tag1/, @{$log_listener->get_log()}), 1);
$event_handler->remove_listener($log_listener);
$connection->io_handle($io_handle);
$connection->close();
$log_handle->close();

# For some reasons deep in the cvs server we need a new connection before we try
# a another recursive command. Bad.
print "# Testing checkout with sticky tag.\n";
chdir($save_dir);
$connection_root = PCVSLib::Root->new(':pserver:user@localhost:' . '8888' . $test_repository);
$connection = PCVSLib::Connection->new($connection_root, $password);
$log_handle = IO::File->new(">>$log_dir/log");
$io_handle = $connection->open();
$connection->io_handle(PCVSLib::LogHandle->new($io_handle, $log_handle));
$client = PCVSLib::Client->new($connection);
chdir("$test_workspace/testmodule");
$tag_listener = LogListener->new();
$event_handler->add_listener($tag_listener);
$tag_command = PCVSLib::TagCommand->new($event_handler);
$tag_command->branch('test_branch');
$tag_command->file_list(['.']);
$client->execute_command($tag_command);
ok($tag_listener->is_success(), 1);
foreach ( @{$tag_listener->get_log()} ) {
    print "# $_\n";
}
$event_handler->remove_listener($tag_listener);

$tag_listener = LogListener->new();
$event_handler->add_listener($tag_listener);
$tag_command = PCVSLib::TagCommand->new($event_handler);
$tag_command->branch('branch_tag2');
$tag_command->file_list(['aaa', 'bbb', 'ccc']);
$client->execute_command($tag_command);
ok($tag_listener->is_success(), 1);
foreach ( @{$tag_listener->get_log()} ) {
    print "# $_\n";
}
$event_handler->remove_listener($tag_listener);

$connection->io_handle($io_handle);
$connection->close();
$log_handle->close();

# For some reasons deep in the cvs server we need a new connection before we try
# a another recursive command. Bad.
print "# Testing checkout of individual files\n";
chdir($save_dir);
system("rm -rf $test_workspace/testmodule");
$connection_root = PCVSLib::Root->new(':pserver:user@localhost:' . '8888' . $test_repository);
$connection = PCVSLib::Connection->new($connection_root, $password);
$log_handle = IO::File->new(">>$log_dir/log");
$io_handle = $connection->open();
$connection->io_handle(PCVSLib::LogHandle->new($io_handle, $log_handle));
$client = PCVSLib::Client->new($connection);
chdir("$test_workspace");
$checkout_listener = LogListener->new();
$event_handler->add_listener($checkout_listener);
$checkout_command = PCVSLib::CheckoutCommand->new($event_handler);
$checkout_command->file_list(['testmodule/aaa', 'testmodule/bbb']);
$client->execute_command($checkout_command);
ok($checkout_listener->is_success(), 1);
ok(-e "$test_workspace/testmodule/CVS/Entries.Static", 1);
$event_handler->remove_listener($checkout_listener);
$connection->io_handle($io_handle);
$connection->close();
$log_handle->close();


# For some reasons deep in the cvs server we need a new connection before we try
# a another recursive command. Bad.
print "# Testing checkout with sticky tag\n";
chdir($save_dir);
system("rm -rf $test_workspace/testmodule");
$connection_root = PCVSLib::Root->new(':pserver:user@localhost:' . '8888' . $test_repository);
$connection = PCVSLib::Connection->new($connection_root, $password);
$log_handle = IO::File->new(">>$log_dir/log");
$io_handle = $connection->open();
$connection->io_handle(PCVSLib::LogHandle->new($io_handle, $log_handle));
$client = PCVSLib::Client->new($connection);
chdir("$test_workspace");
$checkout_listener = LogListener->new();
$event_handler->add_listener($checkout_listener);
$checkout_command = PCVSLib::CheckoutCommand->new($event_handler);
$checkout_command->branch('test_branch');
$checkout_command->file_list(['testmodule']);
$client->execute_command($checkout_command);
ok($checkout_listener->is_success(), 1);
$event_handler->remove_listener($checkout_listener);

print "# Testing status command\n";
chdir("$test_workspace/testmodule");
my $status_command = PCVSLib::StatusCommand->new($event_handler);
my $status_listener = TestListener->new();
$event_handler->add_listener($status_listener);
$status_command->file_list(['.']);
$client->execute_command($status_command);
ok($status_listener->is_success(), 1);
$event_handler->remove_listener($status_listener);

print "# Testing commit command\n";
open(BBB, '>>bbb');
print BBB "next line\n";
print BBB "third line\n";
close(BBB);
open(CCC, '>ccc');
print CCC "first line\n";
print CCC "second line\n";
print CCC "third line\n";
print CCC "fourth line\n";
close(CCC);
my $commit_command = PCVSLib::CommitCommand->new($event_handler);
my $commit_listener = TestListener->new();
$event_handler->add_listener($commit_listener);
$commit_command->file_list(['bbb','ccc']);
$commit_command->comment(["First commit comment", "and already a multiline comment"]);
$client->execute_command($commit_command);
ok($commit_listener->is_success(), 1);
$event_handler->remove_listener($commit_listener);

$connection->io_handle($io_handle);
$connection->close();
$log_handle->close();

# More verification

# For some reasons deep in the cvs server we need a new connection before we try
# a another command. Bad.
$connection_root = PCVSLib::Root->new(':pserver:user@localhost:' . '8888' . $test_repository);
$connection = PCVSLib::Connection->new($connection_root, $password);
$log_handle = IO::File->new(">>$log_dir/log");
$io_handle = $connection->open();
$connection->io_handle(PCVSLib::LogHandle->new($io_handle, $log_handle));
$client = PCVSLib::Client->new($connection);
$status_command = PCVSLib::StatusCommand->new($event_handler);
$status_listener = TestListener->new();
$event_handler->add_listener($status_listener);
$status_command->file_list(['bbb']);
$client->execute_command($status_command);
ok($status_listener->is_success(), 1);
$event_handler->remove_listener($status_listener);
my $ci_directory = PCVSLib::Directory->new("$test_workspace/testmodule");
my $ci_version = $ci_directory->get_entry('bbb')->version();
ok($ci_version eq '1.1.1.1.2.1', 1);
my $ci_time = $ci_directory->get_entry('bbb')->timestamp();
my @stat_buf = stat('bbb');
ok ($ci_time == $stat_buf[9], 1);
ok(-e "$test_workspace/testmodule/CVS/Tag", 1);

print "# Testing update command\n";
my $update_command = PCVSLib::UpdateCommand->new($event_handler);
my $update_listener = TestListener->new();
$event_handler->add_listener($update_listener);
$update_command->file_list(['.']);
$update_command->options(['-A']);
$client->execute_command($update_command);
ok($update_listener->is_success(), 1);
$event_handler->remove_listener($update_listener);
ok(!-e "$test_workspace/testmodule/CVS/Tag", 1);

sleep(1); # timestamps ....
print "# Testing merges\n";
open(BBB, '>>bbb');
print BBB "this will conflict\n";
print BBB "third line\n";
close(BBB);
open(CCC, '>>ccc');
print CCC "fifth line\n";
close(CCC);

my $commit_command1 = PCVSLib::CommitCommand->new($event_handler);
my $commit_listener1 = TestListener->new();
$event_handler->add_listener($commit_listener1);
$commit_command1->file_list(['bbb','ccc']);
$commit_command1->comment(["Commit on trunk"]);
$client->execute_command($commit_command1);
ok($commit_listener1->is_success(), 1);
$event_handler->remove_listener($commit_listener1);

$update_command = PCVSLib::UpdateCommand->new($event_handler);
$update_listener = TestListener->new();
$event_handler->add_listener($update_listener);
$update_command->file_list(['.']);
$update_command->merge1('test_branch');
$client->execute_command($update_command);
ok($update_listener->is_success(), 1);
$event_handler->remove_listener($update_listener);

my $merge_dir = PCVSLib::Directory->new('.');
my $merge_entry = $merge_dir->get_entry('bbb');
my $merge_entry_str = $merge_entry->to_string();
ok( $merge_entry_str =~ /\/Result of merge\+/, 1);
$merge_entry = $merge_dir->get_entry('ccc');
$merge_entry_str = $merge_entry->to_string();
ok( $merge_entry_str =~ /\/Result of merge\//, 1);

my $commit_command2 = PCVSLib::CommitCommand->new($event_handler);
my $commit_listener2 = TestListener->new();
$event_handler->add_listener($commit_listener2);
$commit_command2->file_list(['.']);
$commit_command2->comment(["Commit of merges"]);
$client->execute_command($commit_command2);
# must fail because 'bbb' has not been changed after conflict
ok($commit_listener2->is_success(), 0); 
$event_handler->remove_listener($commit_listener2);

sleep(1);
system("touch bbb");
my $commit_command3 = PCVSLib::CommitCommand->new($event_handler);
my $commit_listener3 = TestListener->new();
$event_handler->add_listener($commit_listener3);
$commit_command3->file_list(['.']);
$commit_command3->comment(["Commit of merges"]);
$client->execute_command($commit_command3);
# now ok, though it will still complain about markers in 'bbb'
ok($commit_listener3->is_success(), 1); 
$event_handler->remove_listener($commit_listener3);

sleep(1);
system("touch ccc");
my $commit_command4 = PCVSLib::CommitCommand->new($event_handler);
my $commit_listener4 = TestListener->new();
$event_handler->add_listener($commit_listener4);
$commit_command4->file_list(['.']);
$commit_command4->comment(["Commit of merges"]);
$client->execute_command($commit_command4);
ok($commit_listener4->is_success(), 1); 
$event_handler->remove_listener($commit_listener4);

$connection->io_handle($io_handle);
$connection->close();
$log_handle->close();

print "# Again check out\n";
chdir($save_dir);
system("rm -rf $test_workspace/testmodule");
$connection_root = PCVSLib::Root->new(':pserver:user@localhost:' . '8888' . $test_repository);
$connection = PCVSLib::Connection->new($connection_root, $password);
$log_handle = IO::File->new(">>$log_dir/log");
$io_handle = $connection->open();
$connection->io_handle(PCVSLib::LogHandle->new($io_handle, $log_handle));
$client = PCVSLib::Client->new($connection);
chdir("$test_workspace");
$checkout_listener = LogListener->new();
$event_handler->add_listener($checkout_listener);
$checkout_command = PCVSLib::CheckoutCommand->new($event_handler);
$checkout_command->file_list(['testmodule']);
$client->execute_command($checkout_command);
ok($checkout_listener->is_success(), 1);
$event_handler->remove_listener($checkout_listener);

print "# rdiff testing\n";
my $rdiff_command = PCVSLib::RDiffCommand->new($event_handler);
my $rdiff_listener = TestListener->new();
$event_handler->add_listener($rdiff_listener);
$rdiff_command->options(['-s']);
$rdiff_command->file_list(['testmodule']);
$rdiff_command->rev1('test_branch');
$rdiff_command->rev2('HEAD');
$client->execute_command($rdiff_command);
ok($rdiff_listener->is_success(), 1);
$event_handler->remove_listener($rdiff_listener);

print "# Testing removal of file\n";
chdir("$test_workspace/testmodule");
my $removal_command = PCVSLib::RemoveCommand->new($event_handler);
my $removal_listener = TestListener->new();
$event_handler->add_listener($removal_listener);
$removal_command->file_list(['aaa','subdir1/ccc']);
$removal_command->options(['-f']);
$removal_command->recursive(0);
$client->execute_command($removal_command);
ok($removal_listener->is_success(), 1);
$event_handler->remove_listener($removal_listener);

my $commit_command5 = PCVSLib::CommitCommand->new($event_handler);
my $commit_listener5 = TestListener->new();
$event_handler->add_listener($commit_listener5);
$commit_command5->file_list(['.']);
$commit_command5->comment(["Commit removed file"]);
$client->execute_command($commit_command5);
ok($commit_listener5->is_success(), 1);
$event_handler->remove_listener($commit_listener5);

$connection->io_handle($io_handle);
$connection->close();
$log_handle->close();

chdir($save_dir);

{   
    local $SIG{CHLD}='IGNORE';
    kill('HUP', $daemon_pid);
}

#### test implementation of a listener

package TestListener;

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{count_}          = 0;
    $self->{message_}        = undef;
    $self->{valid_requests_} = undef;
    $self->{is_success_}     = 0;
    bless ($self, $class);
    return $self;
}

sub is_success 
{
    my $self = shift;
    return $self->{is_success_};
}

sub count 
{
    my $self = shift;
    return $self->{count_};
}

sub get_message
{
    my $self = shift;
    return $self->{message_};
}

sub get_valid_requests
{
    my $self = shift;
    return $self->{valid_requests_};
}

sub notify
{
    my $self  = shift;
    my $event = shift;

    $self->{count_}++;

    if ( $event->isa(PCVSLib::MessageEvent) ) {
        $self->{message_} = $event->get_message();
    }
    if ( $event->isa(PCVSLib::ErrorMessageEvent) ) {
        $self->{message_} = $event->get_message();
    }
    if ( $event->isa(PCVSLib::ValidRequestsEvent) ) {
        $self->{valid_requests_} = $event->get_valid_requests();
    }
    if ( $event->isa(PCVSLib::TerminatedEvent) ) {
        $self->{is_success_} = $event->is_success();
    }
}

package LogListener;

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{is_success_} = 0;
    $self->{log_}        = ();
    bless ($self, $class);
    return $self;
}

sub is_success 
{
    my $self = shift;
    return $self->{is_success_};
}

sub get_log
{
    my $self = shift;
    return $self->{log_};
}

sub notify
{
    my $self  = shift;
    my $event = shift;

    if ( $event->isa(PCVSLib::MessageEvent) ) {
        push(@{$self->{log_}}, $event->get_message());
    }
    if ( $event->isa(PCVSLib::TerminatedEvent) ) {
        $self->{is_success_} = $event->is_success();
    }
}

1;
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
