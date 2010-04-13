<%
    Option Explicit
    Response.Expires = 0
%>

<!-- #include file = "common.inc" -->

<HTML>

<HEAD>
    <META http-equiv="refresh" CONTENT="<% = cnRefreshTime %>">
</HEAD>

<%
    ' get current picture id
    Dim nGIFID

    nGifID = File_getDataVirtual( csFileCurrent, ".", ";" )( 0 )

    ' a new picture was chosen ?
    if nGIFID <> Session( "GIFID" ) then
        ' then store the id of the new picture and show it
        Session( "GIFID" ) = nGIFID
    %>
        <BODY bgcolor="red" onLoad="parent.frame1.location.href='./show.asp?<%= nGIFID %>'">
    <%
    else
    %>
        <BODY bgcolor="green">
    <%
    end if
%>

</BODY>

</HTML>