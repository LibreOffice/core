#!/usr/bin/perl

require "common.pl";

%aRequestMap = common::HTTP_getRequest();

# get new picture
$sCurrPic = $aRequestMap{ "CurrPic" };

@aPictureArray = common::File_read( "picture.txt" );
$nPictureArrayLen = @aPictureArray;

# check if + or - was pressed
if( $aRequestMap{ "Auswahl" } eq "+" )
{
    $sCurrPic = abs( $sCurrPic ) + 1;
}

if( $aRequestMap{ "Auswahl" } eq "-" )
{
    $sCurrPic = abs( $sCurrPic ) - 1;
}

# save picture name
if( (abs( $sCurrPic ) > 0) && ( abs( $sCurrPic ) < ( $nPictureArrayLen ) ) )
{
    open( F_CURRPIC, ">currpic.txt");
    print F_CURRPIC abs( $sCurrPic );
    close( F_CURRPIC );
}

# return to edit page
print "Content-type: text/html\n\n";
print "<HTML>\n<HEAD>\n";
print "<META http-equiv=\"refresh\" CONTENT=\"0 ;URL=editpic.pl\">";
print "<title>savepic.pl</title>";
print "</HEAD>\n";
print "<BODY>\n";
print "</BODY>\n";
print "</HTML>\n";
%>
