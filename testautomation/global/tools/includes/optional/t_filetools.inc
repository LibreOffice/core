'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************

*****

function hHandleInitialDialogs() as integer

    const CFN = "hHandleInitialDialogs::"

    '///<h3>Handle dialogs while opening samples/templates etc.</h3>
    '///<i>The returnvalue is of limited use</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Id of the dialog (integer)</li>
    '///<ul>
    '///+<li>0: No dialog was found</li>
    '///+<li>1: UseOfThisTemplate</li>
    '///+<li>2: StarOfficeCalendar1</li>
    '///+<li>3: NewsletterLayout</li>
    '///+<li>4: PortfolioCurrency</li>
    '///+<li>5: StarOfficeCalendar2</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    
    ' NOTE: The ID of the dialog will be returned but is of limited use
    '       Function returns 0 if no dialogs have been encountered
    
    dim incident as integer
        incident = 0
        
    if ( GVERBOSE ) then printlog( CFN & "Enter" )
    try

        '///+<li>Test for UseOfThisTemplate-dialog</li>
        Kontext "UseOfThisTemplate"
        if ( UseOfThisTemplate.Exists() ) then
            printlog( CFN & "Closing UseOfThisTemplate-dialog" )
            incident = 1
            CancelButton.Click()
        endif
          
        '///+<li>Test for StarOfficeCalendar1-dialog</li>
        Kontext "StarOfficeCalendar1"
        if ( StarOfficeCalendar1.Exists() ) then
            printlog( CFN & "Closing StarOfficeCalendar1-dialog" )
            incident = 2
            Cancel.Click()
        endif

        '///+<li>Test for StarOfficeCalendar2-dialog (This should be idential to StarofficeCalendar1)</li>
        Kontext "StarOfficeCalendar2"
        if ( StarOfficeCalendar2.Exists() ) then
            printlog( CFN & "Closing StarOfficeCalendar2-dialog" )
            incident = 5
            Cancel.Click()
        endif
          
        '///+<li>Test for NewsletterLayout-dialog</li>
        Kontext "NewsletterLayout"
        if ( NewsletterLayout.Exists() ) then
            printlog( CFN & "Closing NewsletterLayout-dialog" )
            incident = 3
            OkButton.Click()
        endif
          
        '///+<li>Test for PortfolioCurrency-dialog</li>
        Kontext "PortfolioCurrency"
        if ( PortfolioCurrency.Exists() ) then
            printlog( CFN & "Closing PortfolioCurrency-dialog" )
            incident = 4
            OKButton.Click()
        endif
    catch
    endcatch
         
    hHandleInitialDialogs() = incident
    if ( GVERBOSE ) then printlog( CFN & "Exit" )
    
    '///</ul>

end function         

'*******************************************************************************


'*******************************************************************************

function hGetFileSizeAsLong( cFileName as string ) as long

    '///<h3>Get the size (long) of a file</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Filename (string)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Size of file (long)</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>    
    
    const CFN = "hGetFileSizeAsLong::"

   ' This function returns the size of a specified file. If it is a directory,
   ' the size will default to -1 (A filesize can never be <0 - hopefully)

   dim iAttrib as integer

   '///+<li>Verify that the given file exists</li>
   if ( app.dir( cFileName ) <> "" ) then

      try
          iAttrib = getattr( cFileName )
      
          '///+<li>Return the size for a file or -1 for a directory</li>
          if ( iAttrib <> 16 and iAttrib <> 17 ) then
             hGetFileSizeAsLong() = filelen( cFileName )
          else
             hGetFileSizeAsLong() = -1
          endif
      catch
          qaerrorlog( CFN & "File was not handled correctly: " & cFileName )
          hGetFileSizeAsLong() = 0
      endcatch

   else

      '///+<li>Return a size of 0 if the file was not found</li>
      hGetFileSizeAsLong() = 0
      warnlog( CFN & "File does not exist: " & cFileName )
                
   endif
   
   '///</ul>

end function

'*******************************************************************************



