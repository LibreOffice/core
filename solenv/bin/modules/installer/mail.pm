#*************************************************************************
#
#   $RCSfile: mail.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2004-07-30 16:37:58 $
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

    push(@message, "Subject: $subject");
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

        if ( $mailinfo ) { send_mail("ERROR", $listener, $mailinfo, $languagestringref, $errordir); }
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

        if ( $mailinfo ) { send_mail("SUCCESS", $listener, $mailinfo, $languagestringref, $completeshipinstalldir); }
        else { installer::exiter::exit_program("ERROR: Could not read mailinfo in list file!", "send_success_mail"); }

    }
}


1;
