#!/usr/bin/perl

print "Content-type: text/html\n\n";

print "<HTML>";

print "<HEAD>";
print "</HEAD>";

print "<FRAMESET ROWS=\"*,2\" BORDER=0 FRAMEBORDER=0 FRAMESPACING=0>";
    print "<FRAME SRC=\"./show.pl\" NAME=\"frame1\" SCROLLING=yes  RESIZE MARGINWIDTH=0 MARGINHEIGHT=0 FRAMEBORDER=0>";
    print "<FRAME SRC=\"./poll.pl\" NAME=\"frame2\" SCROLLING=no NORESIZE MARGINWIDTH=0 MARGINHEIGHT=0 FRAMEBORDER=0>";
print "</FRAMESET>";

print "<NOFRAMES>";

print "<BODY BGCOLOR=\"white\">";
    print "<META HTTP-EQUIV=\"-REFRESH\" CONTENT=\"1;URL=./show.pl\">";
print "</BODY>";

print "</HTML>";