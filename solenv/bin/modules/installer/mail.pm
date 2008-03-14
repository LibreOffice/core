#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: mail.pm,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: rt $ $Date: 2008-03-14 11:38:10 $
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

package installer::mail;

use Net::SMTP;
use installer::converter;
use installer::exiter;
use installer::ziplist;

#########################################
# Sending a mail
#########################################

sub send_mail
{
    my ($message, $listenerstring, $mailinfostring, $languagesref, $destdir) = @_;

    my $listener = installer::converter::convert_stringlist_into_array($listenerstring, ",");
    my $mailinfo = installer::converter::convert_stringlist_into_array($mailinfostring, ",");

    my @listener = ();

    for ( my $i = 0; $i <= $#{$listener}; $i++ ) { push(@listener, ${$listener}[$i]); }
    for ( my $i = 0; $i <= $#{$mailinfo}; $i++ ) { ${$mailinfo}[$i] =~ s/\s*$//g; }

    my $smtphost = ${$mailinfo}[0];
    my $account = ${$mailinfo}[1];
    my $sender = ${$mailinfo}[2];

    if ( ! $smtphost ) { installer::exiter::exit_program("ERROR: Could not read SMTP Host in list file!", "send_mail"); }
    if ( ! $account ) { installer::exiter::exit_program("ERROR: Could not read Account in list file!", "send_mail"); }
    if ( ! $sender ) { installer::exiter::exit_program("ERROR: Could not read Sender in list file!", "send_mail"); }

    my $subject = "";
    my $basestring = $installer::globals::product . " " . $installer::globals::compiler . $installer::globals::productextension . " " . $installer::globals::build. " " . $installer::globals::buildid . " " . $$languagesref . "\n";
    if ( $message eq "ERROR" ) { $subject = "ERROR: $basestring" }
    if ( $message eq "SUCCESS" ) { $subject = "SUCCESS: $basestring" }

    my @message = ();

    my $recipient_string = join ',', @listener;
    push(@message, "Subject: $subject");
    push(@message, "To: $recipient_string");
    push(@message, "\n");
    push(@message, "Located at $destdir");

    if ( $message eq "ERROR" )
    {
        for ( my $j = 0; $j <= $#installer::globals::errorlogfileinfo; $j++ )
        {
            my $line = $installer::globals::errorlogfileinfo[$j];
            $line =~ s/\s*$//g;
            push(@message, $line);
        }
    }

    for ( my $i = 0; $i <= $#message; $i++ ) { $message[$i] = $message[$i] . "\015\012"; }

    my $smtp = Net::SMTP->new( $smtphost, Hello => $account, Debug => 0 );

    # set sender
    $smtp->mail($sender);

    # listener
    my @good_addresses = ();
    $smtp->recipient( @listener, { SkipBad => 1 } );

    # send message
    $smtp->data(\@message);

    # quit server
    $smtp->quit();
}

sub send_fail_mail
{
    my ($allsettingsarrayref, $languagestringref, $errordir) = @_;

    # sending a mail into the error board
    my $listener = "";
    $listener = installer::ziplist::getinfofromziplist($allsettingsarrayref, "fail");

    if ( $$listener )
    {
        my $mailinfo = installer::ziplist::getinfofromziplist($allsettingsarrayref, "mailinfo");

        if ( $$mailinfo ) { send_mail("ERROR", $listener, $mailinfo, $languagestringref, $errordir); }
        else { installer::exiter::exit_program("ERROR: Could not read mailinfo in list file!", "send_fail_mail"); }
    }
}

sub send_success_mail
{
    my ($allsettingsarrayref, $languagestringref, $completeshipinstalldir) = @_;

    # sending success mail
    my $listener = "";
    $listener = installer::ziplist::getinfofromziplist($allsettingsarrayref, "success");

    if ( $$listener )
    {
        my $mailinfo = installer::ziplist::getinfofromziplist($allsettingsarrayref, "mailinfo");

        if ( $$mailinfo ) { send_mail("SUCCESS", $listener, $mailinfo, $languagestringref, $completeshipinstalldir); }
        else { installer::exiter::exit_program("ERROR: Could not read mailinfo in list file!", "send_success_mail"); }

    }
}


1;
