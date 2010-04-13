#!/usr/bin/perl

require "common.pl";

print "Content-type: text/html\n\n";
print "<HTML>\n";

print "<HEAD>\n";
print "<title>editpic.pl</title>";
print "</HEAD>\n";

print "<BODY>\n";
    $sCurrPic = join( "", common::File_read( "currpic.txt" ) );
    @aPictureArray = common::File_read( "picture.txt" );
    $nPictureArrayLen = @aPictureArray;
    print "<FORM action=\"savepic.pl\" method=get>\n";
        if( abs( $sCurrPic ) > 1 )
        {
            print "<INPUT type=submit name=\"Auswahl\" value=\"-\"></INPUT>\n";
        }
        else
        {
            print "<INPUT type=button value=\" \"></INPUT>\n";
        }
        print "<INPUT type=text name=\"CurrPic\" value=\"";
        print $sCurrPic;
        print "\" SIZE=3></INPUT>\n";
        if( abs( $sCurrPic ) < ( $nPictureArrayLen - 1 ) )
        {
            print "<INPUT type=submit name=\"Auswahl\" value=\"+\"></INPUT>\n";
        }
        else
        {
            print "<INPUT type=button value=\" \"></INPUT>\n";
        }
        print "<INPUT type=submit name=\"Auswahl\" value=\"$$2\"></INPUT>\n";
    print "</FORM>\n";
print "</BODY>\n";

print "</HTML>\n";
