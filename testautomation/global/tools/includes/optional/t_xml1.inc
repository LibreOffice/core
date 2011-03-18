'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/************************************************************************
'*
'* Owner : helge.delfs@oracle.com
'*
'* short description : simple XML-Parser for XML-Files from Registration-Database and Routines to work with SAX-Parser in Testtool
'*
'***********************************************************************************
' #1 hXMLGotoElement
' #1 hXMLGetFirstCharsForElement
' #1 ExtractSections
' #1 GetXMLValue
' #1 GetXMLTagValue
' #1 GetXMLValueList
' #1 GetXMLValueType
' #1 GetXMLValueLine
' #1 GetXMLValueGlobal
' #1 GetExtractXMLValue
' #1 GetExtractXMLValueList
' #1 GetExtractXMLValueFromList
' #1 hXMLSeekElementInTree
'\**********************************************************************************

function hXMLGotoElement ( sElementLine as String, optional bSilent as boolean) as boolean
'/// uses SAX Interface in testtool ///'
'///hXMLGotoElement ( sElementLine as String )
'///+Input  : sElementLine  => the tree in DOM as one string seperated with ';'
'///+ - - - - - to be more exact, the Attribute Values to 'oor:name' ///'
'///+ - - - -: bSilent => print warnings? ///'
'///+Output : --
'///+Return : was the Element found?
'///- you can jump directly to the correct entry in the DOM-tree

  Dim sList (50) as String
  Dim i as Integer
  Dim x as Integer
  Dim y as Integer
  Dim n as Integer
  dim iMax as integer
  dim bFound as boolean
  dim bFoundCollect as boolean
  dim iAttrCount as integer
  dim bLocalSilent as boolean
  
  if (isMissing(bSilent)) then
      bLocalSilent = FALSE
  else
      bLocalSilent = bSilent
  endif
  
  bFoundCollect = TRUE
  ExtractSections ( sElementLine, sList () )

   for i=1 to ListCount ( sList () ) ' for every Section
      iMax = SAXGetChildCount()
      x = 0
      bFound = FALSE
'      ' ------------ debug start ----------------------
'      for n = 1 to iMax
'         SAXSeekElement (n)
'         Printlog " " + i + ":(" + n + "/" + iMax + "): '" + SAXGetElementName
'         iAttrCount = SAXGetAttributeCount
'         for y = 1 to iAttrCount
'            Printlog "     " + i + ":" + n + ":(" + y + "/" + iAttrCount + "): '"+SAXGetAttributeName (y) +"' : '"+SAXGetAttributeValue (y) +"' "
'         next y
'         SAXSeekElement (0)
'      next n
'      ' ------------- debug end -----------------------
      while ((bFound = FALSE) AND (x < iMax)) ' compare the VALUE for the ATTRIBUTE 'oor:name' with the wanted ITEM
         inc(x)                               ' do it until it fits; else print warnlog
         SAXSeekElement (x)
         if (SAXGetAttributeValue ("oor:name") <> sList (i)) then
            SAXSeekElement (0)
         else
            bFound = TRUE
         endif
      wend
      if ((bFound = FALSE) AND (bSilent = FALSE)) then
         Warnlog "hXMLGotoElement::ERROR! Element " + i + ": '" + sList (i) + "' not found :-("
      endif
      bFoundCollect = bFound AND bFoundCollect
   next i
   hXMLGotoElement = bFoundCollect
end function
'
'-------------------------------------------------------------------------------
'
function hXMLGetFirstCharsForElement ( sElementLine as String, optional sXMLFile as String, optional bClose as Boolean ) as String
'/// uses SAX Interface in testtool ///'
  Dim bCloseLocal as Boolean
'///hXMLGetFirstCharsForElement ( sElementLine as String, optional sXMLFile as String, optional bClose as Boolean ) as String
'///+Input  : sElementLine  => the tree in DOM as one string seperated with ';'
'///+-        sXMLFile      => ( optional ) if you want to open the DOM for a file you can set here the filename
'///+-        bClose        => ( optional ) if you want to close the DOM after getting the char, you can set this to TRUE ( default is FALSE )
'///+Output : --
'///+Return : the string for the element
'///- you can jump directly to the correct entry in the DOM-tree and get the char for that entry
'///- the DOM is closed after this return

   if IsMissing( sXMLFile ) = FALSE then
      SAXReadFile ( sXMLFile )
   end if

   if IsMissing( bClose ) = TRUE then
      bCloseLocal = FALSE
   else
      bCloseLocal = bClose
   end if

   hXMLGotoElement ( sElementLine )
   SAXSeekElement ( 1 )
   if SAXGetNodeType = NodeTypeCharacter then
      hXMLGetFirstCharsForElement = SAXGetChars
   else
      Warnlog "hXMLGetFirstCharsForElement : the element " + sElementLine + " has no chars"
      hXMLGetFirstCharsForElement = ""
   end if

   if bCloseLocal = TRUE then SAXRelease
end function
'
'-------------------------------------------------------------------------------
'
function ExtractSections ( sInput as String, lsXMLsections() as String ) as Integer
'///Input  : - sInput        => a full section seperated with ';'
'///+-         lsXMLsections => an empty list
'///+Output : the list ( lxXMLsections ) with seperated sectionnames
'///+Return : number of sections

  Dim ii, iLen as Integer
  Dim iList ( 50 ) as String
  Dim bFirstEntry as Boolean
  Dim Dummy as String
  dim sTemp as string
  
   lsXMLsections(0) = 0
   iList (0) = 0
   iLen = len ( sInput )
   bFirstEntry = TRUE

   for ii=1 to ( iLen )
      Dummy = mid ( sInput, ii, 1 )
      if ( mid ( sInput, ii, 1 ) ) = ";" then ListAppend ( iList(), Str ( ii ) )
   next ii

   for ii=1 to ( ListCount ( iList () ) )
      if bFirstEntry = TRUE then
         ListAppend ( lsXMLsections(), Left ( sInput, Val(iList(1))-1 ) )
         bFirstEntry = FALSE
      end if
      Dummy = mid ( sInput, Val(iList(ii))+1, Val(iList(ii+1))-Val(iList(ii))-1 )
      ListAppend ( lsXMLsections(), Dummy )
   next ii

   sTemp = ListCount ( lsXMLsections() )
   ExtractSections = sTemp
   if (sTemp = 0) then
      ListAppend (lsXMLsections(), sInput)
      ExtractSections = 1
   end if
end function
'
'-------------------------------------------------------------------------------
'
function GetXMLValue ( sXMLfile as String, sXMLsectionMaster as String, sXMLsection as String, optional bSilent as Boolean ) as String
'/// Input  : - sXMLfile          => Filename with full path ///
'///+ - - - - - sXMLsectionMaster => the master-section ( mostly in StarOffice the filename without extension ) ///
'///+ - - - - - sXMLsection       => full way to the item ///
'///+ Output : - - ///
'///+ Return : - the value ///
'/// wraper for GetXMLValueGlobal ///'
   if IsMIssing ( bSilent ) <> TRUE then
      GetXMLValue = GetXMLValueGlobal ( sXMLfile, sXMLsectionMaster, sXMLsection, , , , bSilent )
   else
      GetXMLValue = GetXMLValueGlobal ( sXMLfile, sXMLsectionMaster, sXMLsection, , , , )
   end if
end function

function GetXMLTagValue ( sXMLfile as String, sXMLsectionMaster as String, sXMLsection as String, sValue as String ) as String
'/// Input  : - sXMLfile          => Filename with full path ///
'///+ - - - - - sXMLsectionMaster => the master-section ( mostly in StarOffice the filename without extension ) ///
'///+ - - - - - sXMLsection       => full way to the item ///
'///+ Output : - - ///
'///+ Return : - the value ///
'/// wraper for GetXMLValueGlobal ///'
   GetXMLTagValue = GetXMLValueGlobal ( sXMLfile, sXMLsectionMaster, sXMLsection, ,sValue , , )
end function

function GetXMLValueList ( lsXMLValue() as String, sXMLfile as String, sXMLsectionMaster as String, sXMLsection as String ) as String
'/// Input  : - sXMLfile          => Filename with full path ///
'///+ - - - - - sXMLsectionMaster => the master-section ( mostly in StarOffice the filename without extension ) ///
'///+ - - - - - sXMLsection       => full way to the item ///
'///+ Output : - - ///
'///+ Return : - the value ///
'/// wraper for GetXMLValueGlobal ///'
   Dim sLine, sLine1 as String
   Dim i, iCounter as Integer

   sLine = GetXMLValueGlobal ( sXMLfile, sXMLsectionMaster, sXMLsection, ,"cfg:value" , TRUE, )
   if sLine = "" then
      GetXMLValueList = ""
   else
      GetXMLValueList = GetExtractXMLValueList ( lsXMLValue(), sLine, , )
   end if
end function
'
'-------------------------------------------------------------------------------
'
function GetXMLValueType ( sXMLfile as String, sXMLsectionMaster as String, sXMLsection as String, optional sType ) as String
'/// Input  : - sXMLfile          => Filename with full path ///
'///+ - - - - - sXMLsectionMaster => the master-section ( mostly in StarOffice the filename without extension ) ///
'///+ - - - - - sXMLsection       => full way to the item ///
'///+ Output : - - ///
'///+ Return : - the value ///
'/// wraper for GetXMLValueGlobal ///'
 Dim sDummy as String
   if IsMissing ( sType ) = TRUE then
      sDummy = "type"
   else
      sDummy = sType
   end if

   GetXMLValueGlobal ( sXMLfile, sXMLsectionMaster, sXMLsection, sDummy, , , )
   GetXMLValueType = sDummy
end function

function GetXMLValueLine ( sXMLfile as String, sXMLsectionMaster as String, sXMLsection as String ) as String
'/// Input  : - sXMLfile          => Filename with full path ///
'///+ - - - - - sXMLsectionMaster => the master-section ( mostly in StarOffice the filename without extension ) ///
'///+ - - - - - sXMLsection       => full way to the item ///
'///+ Output : - - ///
'///+ Return : - the value ///
'/// wraper for GetXMLValueGlobal ///'
   GetXMLValueLine = GetXMLValueGlobal ( sXMLfile, sXMLsectionMaster, sXMLsection,,, TRUE, )
end function
'
'-------------------------------------------------------------------------------
'
function GetXMLValueGlobal ( sXMLfile as String, sXMLsectionMaster as String, sXMLsection as String, optional sXMLType, optional sXMLTag, optional biWholeLine, optional bSil as Boolean ) as String
'/// uses no SAX Parser : just text search in the file ///'
'/// You can get the value of an item in a XML-file. The value of the item must be set between &lt;value&gt; and &lt;value/&gt;. ///
'///+ The item can be written in one line or in more lines. ///
'/// Input  : - sXMLfile          => Filename with full path ///
'///+ - - - - - sXMLsectionMaster => the master-section ( mostly in StarOffice the filename without extension ) ///
'///+ - - - - - sXMLsection       => full way to the item ///'
'///+ - - - - - optional sXMLType => if you want to get the XML-Type this variable must be set ///'
'///+ - - - - - optional sXMLTag  => if sXMLTag isn't set, "value" is the tag, else you must set the tag here ///'
'///+ Output : - optional sXMLType => if sXMLType is set, it will be returned the Type of the value as string ///'
'///+ Return : - the value of the searched item ///'
'  Dim FileNum as Integer
  Dim Pos, iSec, i, j, iDum as Integer
  Dim MasterSecOK, MasterSecEnd, SecOK, SecEnd, bThrough, bWholeLine as Boolean
  Dim xmlZeile, xmlZeile2, sVariable, sDummy, sDummy2 as String
  Dim lsSecList ( 1000 ) as String
  Dim lsInterim ( 1000 ) as String
  Dim textin as object, sfa as object, xInput as object
  Dim bSilent as Boolean

   if ( IsMissing ( bSil ) ) = TRUE then
      bSilent = FALSE
   else
      bSilent = TRUE
   end if

   if ( IsMissing ( biWholeLine ) ) = TRUE then
      bWholeLine = FALSE
   else
      bWholeLine = TRUE
   end if

   if Dir( sXMLfile ) = "" then
      if bSilent = FALSE then Warnlog "GetXMLValueGlobal(...) : " + sXMLfile + " is missing!"
      exit function
   end if

   MasterSecOK = FALSE : MasterSecEND = FALSE
   SecOK=FALSE : SecEND=FALSE
   bThrough = FALSE
   Pos = 1
   GetXMLValueGlobal = ""

   lsSecList (0) = 0
   lsInterim (0) = 0

   iSec = ExtractSections ( sXMLsection, lsSecList () )
   sVariable = lsSecList (iSec)
   ListDelete ( lsSecList(), iSec )
   iSec = iSec-1
   if iSec = 0 then ListAppend ( lsSecList(), "" )

   textin = createUnoService( "com.sun.star.io.TextInputStream" )
   textin.setEncoding("utf8")
   sfa = createUnoService( "com.sun.star.ucb.SimpleFileAccess" )
   xInput = sfa.openFileRead( sXMLfile )
   textin.setInputStream( xInput )

   do until textin.isEOF()
      xmlZeile = textin.readLine()

      xmlZeile = TrimTab ( Trim ( xmlZeile ) )
      xmlZeile2 = lCASE( xmlZeile )                                      ' control case-insensitiv

      if MasterSecOK = FALSE then                                    ' master-section ( com.sun.star. ... )
         if xmlZeile2= "<" + lCASE( sXMLsectionMaster ) + ">" OR Instr ( xmlZeile2, "<" + lCASE( sXMLsectionMaster ) + " " ) <> 0 then
            MasterSecOK = TRUE
         else
            if xmlZeile2 = "<" + lCASE( sXMLsectionMaster ) + "/>" then
               if bSilent = FALSE then warnlog "GetXMLValueGlobal(...) : '" + sXMLsectionMaster + "'  -> master-section has no entries!"
               exit do
            end if
         end if
      else
         if xmlZeile2= "</" + lCASE( sXMLsectionMaster ) + ">" OR xmlZeile2= "<" + lCASE( sXMLsectionMaster ) + "/>" then
            if bSilent = FALSE then warnlog "GetXMLValueGlobal(...) : '" + lsSecList (Pos) + "'  -> entry could not be found!"
            exit do
         end if

         if ( Instr (xmlZeile2, lCASE ( "<" + lsSecList (Pos)) ) <> 0 AND iSec > 0 ) AND Pos < iSec+1 then
            iDum = Instr ( lsSecList (Pos), " " )
            if iDum <> 0 then lsSecList(Pos) = Left ( lsSecList(Pos), iDum -1 )
            if xmlZeile2 = "<" + lCASE( lsSecList (Pos) ) + "/>" then
               if Pos = iSec then
                  if bSilent = FALSE then Warnlog "GetXMLValueGlobal(...) : '" + svariable + "'  -> entry could not be found"
               else
                  if bSilent = FALSE then Warnlog "GetXMLValueGlobal(...) : '" + lsSecList (Pos) + "'  -> entry could not be found"
               end if
               exit do
            else
               Pos = Pos + 1
            end if
         else
            if Pos > iSec then
               sDummy2 = Mid ( xmlZeile2, 2, len ( svariable ) + 1 )
               if sDummy2 = lCase ( svariable ) + ">" OR sDummy2 = lCase ( svariable ) + " " OR bThrough = TRUE then
                  iDum = Instr ( svariable, " " )                                 ' inserted because an error in GetXMLValueLineExtra 3.11.00 (TZ)
                  if iDum <> 0 then svariable = Left ( svariable, iDum -1 )       ' inserted because an error in GetXMLValueLineExtra 3.11.00 (TZ)
                  sDummy = Mid ( xmlZeile2, len ( xmlZeile2 ) - 1 - len ( svariable), len ( svariable)+1 )
                  if ( bThrough = FALSE AND ( sDummy = "/" + lCase ( svariable ) OR Right (sDummy, 1 ) = "/" ) ) OR ( bThrough = TRUE AND sDummy = "/" + lCase ( svariable ) ) then
                     if ListCount  (  lsInterim () ) = 0 then
                        if bWholeLine = TRUE then
                           GetXMLValueGlobal = xmlZeile
                        else
                           if ( IsMissing ( sXMLTag ) ) = TRUE then
                              if ( IsMissing ( sXMLType ) ) = TRUE then
                                 GetXMLValueGlobal = GetExtractXMLValue ( xmlZeile,, )
                              else
                                 GetXMLValueGlobal = GetExtractXMLValue ( xmlZeile, sXMLType, )
                              end if
                           else
                              if ( IsMissing ( sXMLType ) ) = TRUE then
                                 GetXMLValueGlobal = GetExtractXMLValue ( xmlZeile,, sXMLTag )
                              else
                                 GetXMLValueGlobal = GetExtractXMLValue ( xmlZeile, sXMLType, sXMLTag )
                              end if
                           end if
                        end if
                     else
                        ListAppend ( lsInterim (), xmlZeile2 )
                        if bWholeLine = TRUE then
                           for j=1 to ListCount ( lsInterim () )
                               GetXMLValueGlobal = GetXMLValueGlobal + lsInterim (j)
                           next j
                        else
                           if ( IsMissing ( sXMLTag ) ) = TRUE then
                              if ( IsMissing ( sXMLType ) ) = TRUE then
                                 GetXMLValueGlobal = GetExtractXMLValueFromList ( lsInterim (),, )
                              else
                                 GetXMLValueGlobal = GetExtractXMLValueFromList ( lsInterim (), sXMLType, )
                              end if
                           else
                              if ( IsMissing ( sXMLType ) ) = TRUE then
                                 GetXMLValueGlobal = GetExtractXMLValueFromList ( lsInterim (),, sXMLTag )
                              else
                                 GetXMLValueGlobal = GetExtractXMLValueFromList ( lsInterim (), sXMLType, sXMLTag )
                              end if
                           end if
                        end if
                     end if
                     bThrough = FALSE
                     exit do
                  else
                     if xmlZeile2 <> "" then
                        bThrough = TRUE
                        ListAppend ( lsInterim (), xmlZeile2 )
                     end if
                  end if
               end if
            end if
         end if
      end if
   loop

   xInput.closeInput                                                       '* uno-file-close

   if bSilent = FALSE then
      if MasterSecOK = FALSE then warnlog "GetXMLValueGlobal (...) : '" + sXMLsectionMaster + "'  -> Master-section was not found!"
   end if
   wait 1000
end function
'
'-------------------------------------------------------------------------------
'
function GetExtractXMLValue ( sFullLine as String, optional sXMLType, optional sXMLTag ) as String
'/// Get the value-string, when the text is only in one line. ///'
'/// Input  : - sFullLine          => the whole line out of XML-File ///'
'/// Output : - optional sXMLType => if sXMLType is set, it will be returned the Type of the value as string ///'
'/// Return : - the text between <value> and <value/> ///'
  Dim i, ii, iStart, iEnd as Integer
  Dim sInterim, ssTag as String

   if ( IsMissing ( sXMLTag ) ) = TRUE then
      ssTag = "value"
   else
      ssTag = sXMLTag
   end if

   sInterim = lCase ( sFullLine )

   if InStr ( sInterim, "<" + ssTag + "/>" ) <> 0 then
      GetExtractXMLValue = ""
   else
      iStart = InStr ( sInterim, "<" + ssTag + ">" )
      iEnd = InStr ( sInterim, "</" + ssTag + ">" )
      if iStart <> 0 AND iEnd <> 0 then
         if iStart + len(ssTag) + 2  = iEnd then
            GetExtractXMLValue = ""
         else
            GetExtractXMLValue = Mid ( sFullLine, iStart + len ( ssTag )+2, iEnd - iStart - len ( ssTag ) - 2 )
         end if
      end if
  end if

  if ( IsMissing ( sXMLType ) ) = FALSE then
      sXMLType = lcase (sXMLType)
      ii = InStr ( sInterim, sXMLType + "=" )
      if ii = 0 then ii = InStr ( sInterim, sXMLType + " =" )
      if ii = 0 then ii = InStr ( sInterim, sXMLType + "  =" )

      if ii = 0 then
         sXMLType = ""
      else
         for i=ii to len ( sInterim ) - ii
            if Mid ( sInterim, i, 1 ) = chr (34) then
               iStart=i
               i=1000
            end if
         next i
         for i=(iStart+1) to len ( sInterim ) - (iStart+1)
            if Mid ( sInterim, i, 1 ) = chr (34) then
               iEnd=i
               i=1000
            end if
         next i
         sXMLType = Mid ( sFullLine, iStart+1, iEnd-iStart-1 )
      end if
  end if

end function
'
'-------------------------------------------------------------------------------
'
function GetExtractXMLValueList ( lsXMLValues (), sFullLine as String, optional sXMLType, optional sXMLTag ) as Integer
'/// Get the value-string, when the text is only in one line ///'
'/// Input  : - ///'
'/// Output : - ///'
'/// Return : - ///'
  Dim i, ii, iStart, iEnd as Integer
  Dim sInterim, ssTag as String

   lsXMLValues(0)=0

   if ( IsMissing ( sXMLTag ) ) = TRUE then
      ssTag = "value"
   else
      ssTag = sXMLTag
   end if

   sInterim = lCase ( sFullLine )
   ii = len( sInterim ) / len ( ssTag )                           ' maximal so viele Wiederholungen, wie es sTags gibt

   for i=1 to ii
      if InStr ( sInterim, "<" + ssTag + "/>" ) = 0 then
         iStart = InStr ( sInterim, "<" + ssTag + ">" )
         iEnd = InStr ( sInterim, "</" + ssTag + ">" )
         if iStart <> 0 AND iEnd <> 0 then
            if iStart + len(ssTag) + 2  = iEnd then
               ListAppend ( lsXMLValues(), "" )
               sInterim = Mid ( sInterim, iEnd + len (ssTag)+2, len (sInterim) - iEnd - len (ssTag) - 1 - 2 )
            else
               ListAppend ( lsXMLValues(), Mid ( sInterim, iStart + len(ssTag)+2, iEnd - iStart - len(ssTag)-2  ) )
               sInterim = Mid ( sInterim, iEnd + len (ssTag), len (sInterim) - iEnd - len (ssTag) - 1 )
            end if
         else
            i = ii + 1
         end if
      end if
   next i
   GetExtractXMLValueList = ListCount ( lsXMLValues() )

   if ( IsMissing ( sXMLType ) ) = FALSE then
      sXMLType = lcase (sXMLType)
      ii = InStr ( sInterim, sXMLType + "=" )
      if ii = 0 then ii = InStr ( sInterim, sXMLType + " =" )
      if ii = 0 then ii = InStr ( sInterim, sXMLType + "  =" )

      if ii = 0 then
         sXMLType = ""
      else
         for i=ii to len ( sInterim ) - ii
            if Mid ( sInterim, i, 1 ) = chr (34) then
               iStart=i
               i=1000
            end if
         next i
         for i=(iStart+1) to len ( sInterim ) - (iStart+1)
            if Mid ( sInterim, i, 1 ) = chr (34) then
               iEnd=i
               i=1000
            end if
         next i
         sXMLType = Mid ( sFullLine, iStart+1, iEnd-iStart-1 )
      end if
  end if

end function
'
'-------------------------------------------------------------------------------
'
function GetExtractXMLValueFromList ( lsList() as String, optional sXMLType, optional sXMLTag ) as String
'/// Get the value-string, when the text is in a list ( when the item is written in more than one line ). ///'
'/// Input  : - lsList()          => the list of the whole entry of the xml-item ///'
'/// Output : - optional sXMLType => if sXMLType is set, it will be returned the Type of the value as string ///'
'/// Return : - the text between <value> and <value/> ///'
  Dim i, ii, iStart, iEnd as Integer
  Dim sInterim, sInterim1, ssTag as String

   if ( IsMissing ( sXMLTag ) ) = TRUE then
      ssTag = "value"
   else
      ssTag = sXMLTag
   end if

   for i=1 to ListCount ( lsList() )
      sInterim1 = sInterim1 + lsList(i)
   next i

   sInterim = lCase ( sInterim1 )

   if InStr ( sInterim, "<"+ ssTag +"/>" ) <> 0 then
      GetExtractXMLValueFromList = ""
   else
      iStart = InStr ( sInterim, "<" + ssTag + ">" )
      iEnd = InStr ( sInterim, "</" + ssTag + ">" )
      if iStart <> 0 AND iEnd <> 0 then
         if iStart + len(ssTag) + 2 = iEnd then
            GetExtractXMLValueFromList = ""
         else
            GetExtractXMLValueFromList = Mid ( sInterim1, iStart + len ( ssTag )+2, iEnd - iStart - len ( ssTag ) - 2 )
         end if
      end if
  end if

  if ( IsMissing ( sXMLType ) ) = FALSE then
      sXMLType = lcase (sXMLType)
      ii = InStr ( sInterim, sXMLType + "=" )
      if ii = 0 then ii = InStr ( sInterim, sXMLType + " =" )
      if ii = 0 then ii = InStr ( sInterim, sXMLType + "  =" )

      if ii = 0 then
         sXMLType = ""
      else
         for i=ii to len ( sInterim ) - ii
            if Mid ( sInterim, i, 1 ) = chr (34) then
               iStart=i
               i=1000
            end if
         next i
         for i=(iStart+1) to len ( sInterim ) - (iStart+1)
            if Mid ( sInterim, i, 1 ) = chr (34) then
               iEnd=i
               i=1000
            end if
         next i
         sXMLType = Mid ( sInterim1, iStart+1, iEnd-iStart-1 )
      end if
  end if
end function
' 
'-----------------------------------------------------------------------------
'
function hXMLSeekElementInTree ( sSeekThisNodeXML as STRING ) as BOOLEAN
         hXMLSeekElementInTree = FALSE
' Peter Junge: 2005-07-29         
'///<u><b>Recursion to find XML element</b></u>///
'///Input: 'sSeekThisNodeXML' - XML element to seek, e.g. 'foo:bar'///
'///(A XML DOM has to be loaded before)///
'///Seek begins at current XML pointer///
'///Return: TRUE if element was found, else FALSE///
'///BEHAVIOUR: XML pointer is set to 'foo:bar' if found, if not XML pointer is reset to initial element///
'///NOTE: Currently only the first appearence of 'foo:bar' is found.///
'///NOTE: If e.g. the Nth element should be found you have to modify this function///
'///NOTE: There should be further enhancements possible, e.g. find element with specific attribute///
    dim iIndex as INTEGER
    '///<ul><li>Check if current node matches 'sSeekThisNodeXML'</li>///
    if SAXGetElementName() = sSeekThisNodeXML then
        '///<li>MATCH: Function returns TRUE</li>///
        hXMLSeekElementInTree = TRUE
    else
        '///<li>NO MATCH: LOOKUP if current node has elements</li>///
        for iIndex = 1 to SAXGetChildCount()
            '///<li>-> (Loop) Set pointer on child</li>///
            SAXSeekElement ( iIndex )
            '///<li>-> Check if child is a XML element</li>///
            if SAXGetNodeType() = 556 then
                '///<li>-> RECURSION: function recalls itself for current element</li>///
                if hXMLSeekElementInTree ( sSeekThisNodeXML ) = TRUE then
                   '///<li>Don't forget to pass back the result TRUE to recursions parent</li>///
                   hXMLSeekElementInTree = TRUE
                   '///<li>Exit loop if found</li>///
                   Exit For
                endif
             endif
             '///<li>NO MATCH: Go back to parent in DOM tree</li></ul>///
             SAXSeekElement( 0 )
        next iIndex
    endif
end function

