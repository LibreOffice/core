rem	_______________________________________________________________________________________________________________________________________
rem	Test script for registering or changing filter of our configuration.
rem	_______________________________________________________________________________________________________________________________________

Sub Main
	Dim	xFiterFactory	as object
	Dim	sFilterName		as string

	xFilterFactory = createUNOService("com.sun.star.document.FilterFactory")

	sFilterName = "MeinFilter_5"

rem	AddFilter		( xFilterFactory, sFilterName	)
	ReadFilter		( xFilterFactory, sFilterName	)
rem	QueryFilters	( xFilterFactory				)

	xFilterFactory.flush()

End Sub

rem *************************************************************************************************************
Sub AddFilter( xFilterFactory, sFilterName )
	Dim	lProperties(8)	as new com.sun.star.beans.PropertyValue
	Dim	lUserData  (1)	as string

	lUserData(1) = "Userdata von TestFilter"

	lProperties(0).Name		=	"Type"
	lProperties(0).Value	=	"bmp_MS_Windows"

	lProperties(1).Name		=	"UIName"
	lProperties(1).Value	=	sFilterName

	lProperties(2).Name		=	"DocumentService"
	lProperties(2).Value	=	"com.sun.star.text.TextDocument"

	lProperties(3).Name		=	"FilterService"
	lProperties(3).Value	=	"com.sun.star.comp.framework.TestFilter"

	lProperties(4).Name		=	"Flags"
	lProperties(4).Value	=	256

	lProperties(5).Name		=	"UserData"
	lProperties(5).Value	=	lUserData()

	lProperties(6).Name		=	"FileFormatVersion"
	lProperties(6).Value	=	0

	lProperties(7).Name		=	"TemplateName"
	lProperties(7).Value	=	""

	xFilterFactory.insertByName( sFilterName, lProperties() )
End Sub

rem *************************************************************************************************************
Sub ReadFilter( xFilterFactory, sFilterName )
rem	Dim	lFilters()	as com.sun.star.beans.PropertyValue
	Dim	sOut		as string
	Dim	nCount		as integer

	lProperties = xFilterFactory.getByName( sFilterName )

	sOut = ""
	for nCount=0 to ubound(lProperties()) step 1
		sOut = sOut + lProperties(nCount).Name
		sOut = sOut + " = "
rem #85829# Disable follow if statement to produce bug!
rem		if( lProperties(nCount).Name <> "UserData" ) then
			sOut = sOut + lProperties(nCount).Value
rem		endif
		sOut = sOut + chr(13)
	next nCount

	msgbox sOut
End Sub

rem *************************************************************************************************************
Sub QueryFilters( xFilterFactory )
	Dim	lFilters()	as string
	Dim	sQuery		as string
	Dim	sOut		as string
	Dim	nCount		as integer

	sQuery = "_filterquery_defaultfilter"
rem	sQuery = "_filterquery_textdocument_withdefault"

	lFilters() = xFilterFactory.getByName( sQuery )

	sOut = ""
	for nCount=0 to ubound( lFilters() )
		sOut = sOut + lFilters(nCount) + chr(13)
	next nCount

	msgbox sOut
End Sub
