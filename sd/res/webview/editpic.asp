<%
    Option Explicit
    Response.Expires = 0
    Response.Buffer = True
%>

<!-- #include file = "common.inc" -->


<%
    Dim aPictureArray, nPic, nUpper

    aPictureArray = File_getDataVirtual( csFilePicture, ".", ";" )
    nPic = File_readVirtual( "currpic.txt", "." )
	nUpper = CInt( (UBound(aPictureArray) - 1 ) / 2)
%>

<HTML>
<HEAD>
</HEAD>
<BODY>
    <FORM action="savepic.asp" method=get>
        <%
			if isNumeric(nPic) then
				if  (CInt( nPic ) >= CInt( (UBound(aPictureArray ) - 1 ) / 2 )) then
					nPic = nUpper
				end if
			else
				nPic = nUpper
			end if


            if CInt( nPic ) > 1 then
            %>
                <INPUT type=submit name="Auswahl" value="-"></INPUT>
            <%
            else
            %>
                <INPUT type=button value=" "></INPUT>
            <%
            end if
            %>
            <INPUT type=text name="CurrPic" value="<% = nPic %>" SIZE=3></INPUT>
            <%
            if CInt( nPic ) < CInt( nUpper ) then
            %>
                <INPUT type=submit name="Auswahl" value="+"></INPUT>
            <%
            else
            %>
                <INPUT type=button value=" "></INPUT>
            <%
            end if
        %>
        <INPUT type=submit name="Auswahl" value="$$2"></INPUT>
    </FORM>
</BODY>
</HTML>
