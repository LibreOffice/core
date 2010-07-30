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
'* owner : helge.delfs@oracle.com
'*
'* short description : XML search routines II
'*
'************************************************************************
'*
' #1 GetXMLValueLineExtra     ' DEPRECATED depending on t_xml1.inc::GetXMLValueGlobal
' #1 XMLWellFormed            ' Checks the well formness of a XML file. 
' #1 GetXMLValue2             ' OBSOLETE: XML search routine (as TT has no SAX included we have used that rountine)
' #1 GetBodiesItemStyleName   ' DEPRECATED used by ../xml/level1/inc/sxw7_02.inc
' #1 GetLineInXMLBody         ' DEPRECATED used by ../xml/level1/inc/sxw7_02.inc and ../sxw7_03.inc
' #1 GetItemStyleName         ' DEPRECATED used by ../xml/level1/inc/sxw7_01.inc
' #1 GetXMLElementPath        ' Gets the elementpath in a DOM tree (mostly used for [automatic-]styles)
' #1 fWhereIsXMLElementInBody ' Gets the elementpath in a DOM tree in the second level (mostly body elements)
'*
'\***********************************************************************

function GetXMLValueLineExtra ( sXMLfile as String, sXMLsectionMaster as String, sXMLsection as String, sGroupTyp as String, sGroupName as String ) as String
'///+ Input:<ul><li>sXMLfile =&gt; Filename with full path</li>
'///+ <li>sXMLsectionMaster =&gt; The master-section (mostly in OpenOffice.org the filename without extension)</li>
'///+ <li>sXMLsection =&gt; Full way to the item</li>
'///+ <li>sGroupTyp =&gt; First entry after tag</li>
'///+ <li>sGroupName =&gt; Value of first entry</li></ul>
   GetXMLValueLineExtra = GetXMLValueGlobal ( sXMLfile, sXMLsectionMaster, sXMLsection + " " + sGroupTyp + "=" + Chr(34) + sGroupName + Chr(34),,, TRUE )
end function

'-------------------------------------------------------------------------

function GetXMLItemInstance ( sXMLfile as String, sXMLsectionMaster as String, sXMLsection as String ) as String
'/// Input:<ul><li>sXMLfile =&gt; Filename with full path</li>
'///+ <li>sXMLsectionMaster =&gt; The master-section (mostly in OpenOffice.org the filename without extension)</li>
'///+ <li>sXMLsection =&gt; Full way to the item</li></ul>
  Dim sLine, sLine2 as String
  Dim iStart, iEnd, i, iStr, iLen as Integer

   sLine = GetXMLValueLine ( sXMLfile, sXMLsectionMaster, sXMLsection )
   sLine2 = lcase ( sLine )
   iStr = Instr ( sLine2, "instance" )
   iLen = len ( sLine2 )
   iStart = 0

   if iStr=0 then
      GetXMLItemInstance = "false"
      exit function
   else
      for i=iStr to iLen
         if iStart = 0 then
            if Mid ( sLine2, i, 1 ) = Chr(34) then iStart = i
         else
            if Mid ( sLine2, i, 1 ) = Chr(34) then
               iEnd = i
               i= iLen + 1
            end if
         end if
      next i
   end if
   if iStart = 0 then
      GetXMLItemInstance = ""
   else
      GetXMLItemInstance = Mid ( sLine, iStart+1, iEnd - iStart - 1 )
   end if
end function

'-------------------------------------------------------------------------

function GetXMLItemEncoding ( sXMLfile as String, sXMLsectionMaster as String, sXMLsection as String ) as String
'/// Input: <ul><li>sXMLfile =&gt; Filename with full path</li>
'///+ <li>sXMLsectionMaster =&gt; The master-section (mostly in OpenOffice.org the filename without extension)</li>
'///+ <li>sXMLsection =&gt; Full way to the item</li></ul>
  Dim sLine, sLine2 as String
  Dim iStart, iEnd, i, iStr, iLen as Integer

   sLine = GetXMLValueLine ( sXMLfile, sXMLsectionMaster, sXMLsection )
   sLine2 = lcase ( sLine )
   iStr = Instr ( sLine2, "encoding" )
   iLen = len ( sLine2 )
   iStart = 0

   if iStr=0 then
      GetXMLItemEncoding = "false"
      exit function
   else
      for i=iStr to iLen
         if iStart = 0 then
            if Mid ( sLine2, i, 1 ) = Chr(34) then iStart = i
         else
            if Mid ( sLine2, i, 1 ) = Chr(34) then
               iEnd = i
               i= iLen + 1
            end if
         end if
      next i
   end if
   if iStart = 0 then
      GetXMLItemEncoding = ""
   else
      GetXMLItemEncoding = Mid ( sLine, iStart+1, iEnd - iStart - 1 )
   end if
end function

'-------------------------------------------------------------------------

function XMLWellFormed ( sFileName as String, optional bDebug as Boolean ) as Boolean
'/// Input: File name as string
'/// <i>(obsolete: Debug)</i>
'/// Return: TRUE or FALSE
 Dim InputstreamXML as string
  XMLWellFormed = FALSE
  if IsMissing(bDebug) = FALSE then
   warnlog "Debugmode 'XMLWellFormed' is obsolete. FUNCTION is now a native TestTool function!"
  end if
  InputstreamXML = SAXCheckWellFormed(sFileName)
  if InputstreamXML <> "" then
    warnlog "Problem was found: " & InputstreamXML
   else
    printlog "File: '" & sFileName & "' is well formed."
    XMLWellFormed = TRUE
  end if
end function

'-------------------------------------------------------------------------

function GetXMLValue2 ( sXMLfile as String, sXMLsectionMaster as String, sXMLsection as String ) as String
'/// Input: <ul><li>sXMLfile =&gt; Filename with full path</li></ul>
 Dim lsList(10) as String
 Dim sInterim, sInterim2 as String
 Dim i, ii, iLen, ibegin ,iEnd as Integer

   sInterim = GetXMLValueLine ( sXMLfile, sXMLsectionMaster, sXMLsection,,, )
   if sInterim = "" then
      GetXMLValue2 = ""
      exit function
   end if

   i = ExtractSections ( sXMLsection, lsList() )
   sInterim2 = lsList(i)
   iLen = len ( sInterim )
   ii = len ( sInterim2 ) + 1

   for i=ii to iLen
      if mid( sInterim, i, 1 ) = ">" then
         iBegin = i+1
         i=iLen+1
      end if
   next i
   iEnd = ( iLen - ii - 1 ) - iBegin
   GetXMLValue2 = Mid ( sInterim, iBegin, iEnd )
end function

'-------------------------------------------------------------------------

function GetBodiesItemStyleName ( AXMLfile as string , WhichItem as string , HowOften as integer , OPTIONAL B ) as string
' Author: Joerg Sievers
'/// With GetBodiesItemStyleName you can get the NAME of a STYLE from
'///+ a item in the BODY of a OpenOffice.org XML-file.
'/// <blockquote>OPTIONAL PARAMETER
'///+ If there are more than one &quot;style-name&quot; tags in ONE line, you
'///+ have to use an optional parameter.
'/// <i>see also</i>:<ul>
'///+ <li>GetXMLValueLineExtra</li></ul>
'/// <u>simple Example</u>:
'///+ String = GetBodiesStyleName (&quot;example.sxc&quot;) , &quot;table:table-row&quot; , 2)
'///+ Return: The second STYLE-NAME of the &apos;table-row&apos;-tag in the office:body
'/// <u>Example with optional parameter</u>: 
'///+ XML-line you want to parse for the text:span style-name and it is the second
'///+ text:span-attribute in the office:body-section:
'///+ <blockquote>
'///+ &lt;text:p text:style-name=&quot;P1&quot;&gt;&lt;text:span text:style-name=&quot;T1&quot;&gt;The first text&lt;/text:span&gt;&lt;/text:p&gt;
'///+ &lt;text:p text:style-name=&quot;P4&quot;&gt;&lt;text:span text:style-name=&quot;T4&quot;&gt;Just a text&lt;/text:span&gt;&lt;/text:p&gt;
'///+ </blockquote> 
'///+ then you have to use:
'///+ String = GetBodiesStyleName (&quot;example.sxc&quot;) , &quot;table:table-row&quot; , 2 , 1) 
'///+ The first ineteger (2) is for the second <text:span-entry in the file.
'///+ The OPTIONAL second integer is the 'ONE' AFTER the first tag in the same line.
  Dim FileNum as integer
  Dim XMLRawLine as string
  Dim XMLCLearedLine as string
  Dim a as integer
  Dim i as integer
  Dim FoundEntry as boolean
  Dim DelLeft as integer
  Dim ItemPosInString as integer
  Dim XMLCLearedLineWithoutLeft as string
  Dim DelRight as integer
  Dim XMLCLearedAndSeperatedLine as string

   if Dir ( AXMLfile ) = "" then
      warnlog "GetBodiesItemStyleName(...) : '" & AXMLfile & "' is missing!"
      exit function
   end if

  WhichItem = "<" & WhichItem

  a = 0
  FoundEntry = FALSE

  FileNum = FreeFile

   Open AXMLfile For Input As #FileNum

    Do until EOF(#FileNum) = TRUE OR FoundEntry = TRUE

     Line input #FileNum, XMLRawLine
'    deleting tabs and spaces
     XMLCLearedLine = TrimTab ( Trim ( XMLRawLine ) )
'    jumping to the office:body
     if InStr(XMLCLearedLine , "<office:document-") <> 0 then

        Do until EOF(#FileNum) = TRUE OR FoundEntry = TRUE
         Line input #FileNum, XMLRawLine
         XMLCLearedLine = TrimTab ( Trim ( XMLRawLine ) )
'        if the count of the item is the same as the one searching for...
         ItemPosInString = InStr(XMLCLearedLine , WhichItem)
           if ItemPosInString <> 0 then
'            warnlog "Debug: ItemPosInString = '" & ItemPosInString & "'"
'           Is there more than one time the 'WhichItem' in this line?
'           (MUST BE GIVEN BY OPTIONAL PARAMETER!)
             if IsMissing(B) = FALSE then
                 For i = 1 to B
                    DelLeft = InStr(XMLCLearedLine ,  "style-name=" &  CHR$(34))
                    XMLCLearedLineWithoutLeft = Mid(XMLCLearedLine, DelLeft+12)
                    XMLCLearedLine = XMLCLearedLineWithoutLeft
                 Next i
             end if
             a = a+1
             if a=HowOften then
'                searching for exakt attribute stylename="
                 DelLeft = InStr(XMLCLearedLine ,  "style-name=" &  CHR$(34))
'                extrcting, stripping all things after the style-name-attribute (=12 chars)
                 XMLCLearedLineWithoutLeft = Mid(XMLCLearedLine, DelLeft+12)
'                extracting the real name without the "
                 DelRight = InStr(XMLCLearedLineWithoutLeft ,  CHR$(34))
'                stripping it
                 XMLCLearedAndSeperatedLine = Mid(XMLCLearedLineWithoutLeft, 1 , DelRight-1)
                 GetBodiesItemStyleName = GetBodiesItemStyleName+XMLCLearedAndSeperatedLine
                 FoundEntry = TRUE
              end if
         end if
        loop
     end if

    loop
   Close #FileNum
end function

'-------------------------------------------------------------------------

function GetLineInXMLBody ( AXMLfile as string , WhichItem as string , HowOften as integer) as string
'Author: Joerg Sievers
'/// With this function you can extract a whole line in &lt;office:body&gt;
'///+ of a XML document. It is important to give this routine the
'///+ correct <ITEM and the count in <office:body>.
'/// <u>simple Example</u>:
'///+ We want to find the 2nd (!) table:table-row item
'///+ &lt;table:table-row table:style-name=&quot;ro2&quot; table:visibility=&quot;collapse&quot;&gt; 
'///+ String = GetLineInXMLBody(gOfficePath & ConvertPath(&quot;Content.xml&quot;) , &quot;table:table-row&quot; , 2)
'///+ Return: The whole line of the second &apos;table:table-row&apos;-item.
  Dim FileNum as integer
  Dim XMLRawLine as string
  Dim XMLCLearedLine as string
  Dim a as integer
  Dim FoundEntry as boolean
  Dim ItemPosInString as integer

   if Dir ( AXMLfile ) = "" then
      warnlog "GetLineInXMLBody(...) : '" & AXMLfile & "' is missing!"
      exit function
   end if

  WhichItem = "<" & WhichItem

  a = 0
  FoundEntry = FALSE

  FileNum = FreeFile

   Open AXMLfile For Input As #FileNum

    Do until EOF(#FileNum) = TRUE OR FoundEntry = TRUE

     Line input #FileNum, XMLRawLine
'    deleting tabs and spaces
     XMLCLearedLine = TrimTab ( Trim ( XMLRawLine ) )      
'    jumping to the office:body
     if InStr(XMLCLearedLine , "office:body") <> 0 then
        Do until EOF(#FileNum) = TRUE OR FoundEntry = TRUE
         Line input #FileNum, XMLRawLine
         XMLCLearedLine = TrimTab ( Trim ( XMLRawLine ) )         
'        if the count of the item is the same as the one searching for...
         ItemPosInString = InStr(XMLCLearedLine , WhichItem)
           if ItemPosInString <> 0 then
             a = a+1
             if a=HowOften then
                 GetLineInXMLBody = XMLCLearedLine
'                 printlog "Debug from 'GetLineInXMLBody'-function: '" & XMLCLearedLine & "'"
                 FoundEntry = TRUE
              end if
         end if
        loop
     end if
    loop
   Close #FileNum
end function
'
'-------------------------------------------------------------------------------
'
function GetItemStyleName ( sMainElement$ , sUsedElement$ , sElement$ , sCount as integer , sStyleName$ ) as string
'Author: Joerg Sievers
'/// With GetItemStyleName you can get the NAME of a STYLE in ANY
'///+ Element in a main element (like office:body-content) of a OpenOffice.org XML-file.
'/// ATTENTION: 
'///+ If the second parameter is the SAME as the third one, you are not going into the
'///+ third area. You will stay in the DOM in the second, where e.g. table:table element could
'///+ be found. 
'/// <u>simple Example</u>: 
'///+ String = GetItemStyleName ( &quot;office:body&quot;, &quot;table:table&quot;, &quot;table:table-row&quot; , 3 , &quot;table:style-name&quot; )
'///+ Return: The (attribute) name of the 3rd table-row node in a Writer document.
'///+ With this name you can search in the style-section for the correct values.
  Dim InputstreamXML as integer
  Dim i as integer
  Dim a as integer
  Dim xElementName as string
   'Read the file and go to the main DOM node
   SAXSeekElement(1)
' If you need a debug mode, enable the printlog entries
' -----------------------------------------------------
'   printlog " +-- function: GetItemStyleName ---------------------------"
'   printlog " | Main Node         : " & SAXGetElementName
   'Go to the main element (like office:body, office:script, office:automatic-styles, etc.)
   SAXSeekElement(sMainElement$)
'   printlog " | Main Element      : " & sMainElement$
   if sUsedElement$ <> sElement$ then
    SAXSeekElement(sUsedElement$)
'    printlog " | Used Element      : " & sUsedElement$
   end if
   InputstreamXML = SaxGetChildCount
'   printlog " | Count of Children : " & InputstreamXML
   for i = 1 to InputstreamXML
    SAXSeekElement(i)
    if SAXGetNodeType = NodeTypeElement then
      xElementName = SAXGetElementName
      if xElementName = sElement$ then
        a= a+1
'        printlog "("& i & " / " & a & ") Element:" & xElementName
        if a = sCount then
         GetItemStyleName = SAXGetAttributeValue(sStyleName$)
'         printlog " | Name of Element   : " & GetItemStyleName
         exit for
        end if
      end if
      SAXSeekElement(0)
    end if
   next i
'   printlog " +---------------------------------------------------------"
end function

'-------------------------------------------------------------------------

function GetXMLElementPath  ( sMainElement$ , sUsedElement$ , sStyleName$, sStyleNameValue$ ) as string
'Author: Joerg Sievers
'///+ Returns the path (in a DOM tree) for an exact named element.
'///+ With this string it is possible to navigate easily to a named
'///+ element with <i>SAXSeekElement</i>-function.
  Dim InputstreamXML as integer
  Dim i as integer
  Dim xAttributeValue as string
  Dim a as integer
  Dim xElementName as string
   'Read the file and go to the main DOM node
   SAXSeekElement("/")
   SAXSeekElement(1)
   InputstreamXML = SaxGetChildCount
' If you need a debug mode, enable the printlog entries
' -----------------------------------------------------
'   printlog " +-- function: GetXMLElementPath --------------------------"
'   printlog " | Main Node         : " & SAXGetElementName
   'Go to the main element (like office:body, office:script, office:automatic-styles, etc.)
   SAXSeekElement(sMainElement$)
'   printlog " | Main Element      : " & sMainElement$
   InputstreamXML = SaxGetChildCount
'   printlog " | Count of Children : " & InputstreamXML
   for i = 1 to InputstreamXML
     SAXSeekElement(i)
     xElementName = SAXGetElementName(i)
     if xElementName = sUsedElement$ then
       xAttributeValue = SAXGetAttributeValue(sStyleName$)
       if xAttributeValue = sStyleNameValue$ then
'         printlog " | Elementname (" & i & ")   : " & xElementName
'         printlog " | Attribute value   : " & xAttributeValue
         GetXMLElementPath = SAXGetElementPath
         exit for
       end if
     end if
    SAXSeekElement(0)
   next i
'   printlog " +---------------------------------------------------------"
end function

'-------------------------------------------------------------------------

function fWhereIsXMLElementInBody ( sSubDocumentRootElement as string , sDocumentRootElement as string , sWhichElement as string , OPTIONAL A ) as string
Dim iXMLElements as integer
Dim k as integer
Dim iXMLElementsInSecondLayer as integer
Dim i as integer
'/// A function to parse a XML DOM of an office document and return the <i>Elementpath</i>
'///+  of an element where you can search for in the <u>second level</u>. 
SAXSeekElement(sSubDocumentRootElement)
'/// Input:<ol><li>Which element to be searched for</li>
'///+ <li><i>SubDocumentRootElement</i>:
'///+ <ul><li>office:document-meta</li>
'///+ <li>office:document-styles</li>
'///+ <li>office:document-content</li>
'///+ <li>office:document-settings</li></ul></li>
SAXSeekElement(sDocumentRootElement)
'///+ <li><i>DocumentRootElement</i>:
'///+ <ul><li>office:meta</li>
'///+ <li>office:settings</li>
'///+ <li>office:scripts</li>
'///+ <li>office:font-decls</li>
'///+ <li>office:styles</li>
'///+ <li>office:automatic-styles</li>
'///+ <li>office:master-styles</li>
 '///+ <li>office:body</li></ul></li></ol>
iXMLElements = SAXGetChildCount
 'If you need debug information, change next line value to TRUE
 EnableQAErrors = FALSE 
 QAErrorLog "Debug: t_xml2.inc::fWhereIsXMLElementInBody::iXMLElements = "& iXMLElements 
for i = 1 to iXMLElements
  if SAXHasElement(i) = TRUE then
   SAXSeekElement(i)
    QAErrorLog "Debug: t_xml2.inc::fWhereIsXMLElementInBody::SAXSeekelement(i) [" & i & "] = +- "& SAXGetElementname(i)
    iXMLElementsInSecondLayer = SAXGetChildCount
    QAErrorLog "Debug: t_xml2.inc::fWhereIsXMLElementInBody::iXMLElementsInSecondLayer [" & i & "] = +- "& iXMLElementsInSecondLayer
    if iXMLElementsInSecondLayer <> 0 then
      for k = 1 to iXMLElementsInSecondLayer     
       SAXSeekElement(k)
        if SAXGetNodeType = NodeTypeElement then
         QAErrorLog "Debug: t_xml2.inc::fWhereIsXMLElementInBody::SAXGetElementName(a) [" & k & "] =  +- "& SAXGetElementName(k)
         if IsMissing(A) then
           if SAXGetElementName(k) = sWhichElement then
               fWhereIsXMLElementInBody = SAXGetElementPath
               QAErrorLog "Debug: t_xml2.inc::fWhereIsXMLElementInBody =" & fWhereIsXMLElementInBody            
               exit function
           end if
          else
           if SAXGetElementName((k-1)+A) = sWhichElement then              
               SAXSeekElement(0)
               SAXSeekElement(sWhichElement , A)
               QAErrorLog "Debug: t_xml2.inc::fWhereIsXMLElementInBody::SAXGetChildCount =" & SAXGetChildCount
               fWhereIsXMLElementInBody = SAXGetElementPath               
               QAErrorLog "Debug: t_xml2.inc::fWhereIsXMLElementInBody =" & fWhereIsXMLElementInBody              
              exit function
           end if
         end if
        end if
       SAXSeekElement(0)
      next k     
    end if
   SAXSeekElement(0)
  end if
next i 
end function
'-------------------------------------------------------------------------

