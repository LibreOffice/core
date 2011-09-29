#!/usr/bin/perl

require "common.pl";

print "Content-type: text/html\n\n";

# get current and next picture
$sCurrPic = join( "", common::File_read( "currpic.txt" ) );
@aPictureArray = common::File_read( "picture.txt" );

# not last picture or wrong input ?
if( abs( $sCurrPic ) > 0 )
{
    $nCurrPic = abs( $sCurrPic );
    $nPictureArrayLen = @aPictureArray;
    if( $nCurrPic < $nPictureArrayLen )
    {
        $sPictureName1 = ( split( ";", @aPictureArray[ $nCurrPic ] ) )[ 1 ];
        $sPictureName2 = ( split( ";", @aPictureArray[ $nCurrPic + 1 ] ) )[ 1 ];
    }
    else
    {
        $sPictureName1 = ( split( ";", @aPictureArray[ $nCurrPic ] ) )[ 1 ];
        $sPictureName2 = $sPictureName1;
    }
}

print "<HTML>";

print "<HEAD>";
    print "<TITLE>$$1</TITLE>";
print "</HEAD>";

print "<BODY bgcolor=\"white\">";
    print "<P ALIGN=CENTER><IMG src=\"" . $sPictureName1 . "\" width=$$4 height=$$5 border=0>";
    print "<P><IMG src=\"" . $sPictureName2 . "\" width=1 height=1 border=0>";
print "</BODY>";

print "</HTML>";