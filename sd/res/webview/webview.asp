<%
    Option Explicit
    Response.Expires = 0
%>

<!-- #include file = "common.inc" -->

<%
    Session( "GIFID" ) = File_getDataVirtual( csFileCurrent, ".", ";" )( 0 )
%>

<HTML>

<HEAD>
</HEAD>

<FRAMESET ROWS="*,2" BORDER=0 FRAMEBORDER=0 FRAMESPACING=0>
	<FRAME SRC="./show.asp" NAME="frame1" SCROLLING=yes  RESIZE MARGINWIDTH=0 MARGINHEIGHT=0 FRAMEBORDER=0>
	<FRAME SRC="./poll.asp" NAME="frame2" SCROLLING=no NORESIZE MARGINWIDTH=0 MARGINHEIGHT=0 FRAMEBORDER=0>
</FRAMESET>

<NOFRAMES>

<BODY BGCOLOR="white">
    <META HTTP-EQUIV="-REFRESH" CONTENT="1;URL=./show.asp">
</BODY>

</HTML>