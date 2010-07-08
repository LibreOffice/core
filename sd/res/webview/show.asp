<%
    Option Explicit
    Response.Expires = 0
%>

<!-- #include file = "common.inc" -->

<%
    Dim sGifName1, sGifName2, nGifID, aPictureArray

    nGifID      = Session( "GIFID" )

    ' get current and next picture
    aPictureArray = File_getDataVirtual( csFilePicture, ".", ";" )

    ' not last picture or wrong input ?
    If CInt( nGifID ) < UBound( aPictureArray ) / 2 Then
        sGifName1 = aPictureArray( ( nGifID - 1 ) * 2 + 1 )
        sGifName2 = aPictureArray( ( nGifID ) * 2 + 1 )
    Else
        nGifID = CInt( UBound( aPictureArray ) / 2 )
        sGifName1 = aPictureArray( ( nGifID - 1 ) * 2 + 1 )
        sGifName2 = sGifName1
    End If
%>

<HTML>

<HEAD>
    <TITLE>$$1</TITLE>
</HEAD>

<BODY bgcolor="white">
	<table width=100% height=99%>
	<tr valign=center><td align=center>
		<IMG src="<% = sGifName1 %>" width=$$4 height=$$5 border=0>
		<br><IMG src="<% = sGifName2 %>" width=1 height=1 border=0>
	</td></tr>
	</table>
</BODY>

</HTML>