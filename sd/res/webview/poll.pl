#!/usr/bin/perl

require "common.pl";

print "Content-type: text/html\n\n";
print "<HTML>";

# get current and last shown picture id
$sCurrPic = join( "", common::File_read( "currpic.txt" ) );

%aRequestMap = common::HTTP_getRequest();
$sLastPic = $aRequestMap{ "LastPic" };

print "<HEAD>";
    print "<META http-equiv=\"refresh\" CONTENT=\"" . $common::REFRESH_TIME . "; URL=poll.pl?LastPic=" . $sCurrPic . "\">";
print "</HEAD>";

#' a new picture was chosen ?
if( $sLastPic ne $sCurrPic )
{
    # then show the new picture
    print "<BODY bgcolor=\"red\" onLoad=\"parent.frame1.location.href='./show.pl?" . $sCurrPic . "'\">";
}
else
{
    # otherwise do nothing
    print "<BODY bgcolor=\"green\">";
}

print "</BODY>";

print "</HTML>";