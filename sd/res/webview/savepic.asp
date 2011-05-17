<%
    Option Explicit
    Response.Expires = 0
    Response.Buffer = True
%>

<!-- #include file = "common.inc" -->

<%
    ' get new picture
    Dim sCurrPic,aPictureArray, nUpper, nCurrPic

    aPictureArray = File_getDataVirtual( csFilePicture, ".", ";" )
    nUpper = CInt( (UBound(aPictureArray) - 1 ) / 2)

    sCurrPic = Request( "CurrPic" )

    ' check if + or - was pressed
    select case Request( "Auswahl" )
        case "+"
            if isNumeric( sCurrPic ) then
                sCurrPic = CStr( CLng( sCurrPic ) + 1 )
            end if
        case "-"
            if isNumeric( sCurrPic ) then
                sCurrPic = CStr( CLng( sCurrPic ) - 1 )
            end if
    end select

    ' save picture name
	if isNumeric( sCurrPic ) then
		if (CInt( sCurrPic ) > 0) and ( CInt( sCurrPic ) <= nUpper ) then
			call File_writeVirtual( "currpic.txt", ".", sCurrPic )
		end if
	end if

    ' return to edit page
    Response.Redirect( "./editpic.asp" )
%>
