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
'* owner : marc.neumann@oracle.com
'*
'* short description : Dbase function test
'*
'\***********************************************************************
global gFileName as string
sub db_DbaseFunction(sfilename as string)

'/// this test need a datasource TT1 and the table TT_Func1 from dbaccess/optional/input/dbase_datasource/TT_Func1.dbf
'/// see http://dba.openoffice.org/specifications/file_based_functions.html for details for each function

    gFileName = sfilename    

    call tLOWER()
    call tLCASE()
    call tUPPER()
    call tUCASE()
    call tASCII()
    call tLENGTH()
    call tOCTET_LENGTH()
    call tCHAR_LENGTH()
    call tCHARACTER_LENGTH()
    call tCHAR()
    call tCONCAT()
    call tLOCATE()
    call tSUBSTRING1()
    call tSUBSTRING2()
    call tSUBSTRING3()    
    call tLTRIM()
    call tRTRIM()
    call tSPACE()
    call tREPLACE()
    call tREPEAT()
    call tINSERT()
    call tLEFT()
    call tRIGHT()    
'-------------------------------------------------------------------------        
'Numeric fucntions
'-------------------------------------------------------------------------       
    call tABS()
    call tSIGN()   
    call tMOD()
    call tFLOOR()
    call tCEILING()
    call tROUND1()
    call tROUND2()
    call tEXP()  
    call tLN()
    call tLOG1()
    call tLOG2()    
    call tLOG10()
    call tPOWER()       
    call tSQRT()       
    call tPI()
    call tCOS()
    call tSIN()
    call tTAN()
    call tACOS()
    call tASIN()    
    call tATAN()        
    call tATAN2()        
    call tDEGREES()        
    call tRADIANS()
'-------------------------------------------------------------------------        
'Date / Time fucntions
'-------------------------------------------------------------------------
    call tDAYOFWEEK()
    call tDAYOFMONTH()
    call tDAYOFYEAR()
    call tMONTH()
    call tDAYNAME()
    call tMONTHNAME()
    call tQUARTER()
    call tWEEK()
    call tWEEK2()
    call tYEAR()
    call tHOUR()
    call tMINUTE()
    call tSECOND()
    call tCURDATE()
    call tCURTIME()
    call tNOW()
    
end sub
'-------------------------------------------------------------------------
testcase tLOWER()
    
    sInit(gFileName,"TT_Func1")    
    sSet("LOWER(f_text)")    
    if fExcecute then
         sCheckValue("text")
    endif    
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tLCASE()
    
    sInit(gFileName,"TT_Func1")    
    sSet("LCASE(f_text)")    
    if fExcecute then
        sCheckValue("text")
    endif    
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tUPPER()
    
    sInit(gFileName,"TT_Func1")
    sSet("UPPER(f_text)")    
    if fExcecute then
        sCheckValue("TEXT") 
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tUCASE()
    
    sInit(gFileName,"TT_Func1")
    sSet("UCASE(f_text)")    
    if fExcecute then
        sCheckValue("TEXT")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tASCII()
    
    sInit(gFileName,"TT_Func1")
    sSet("ASCII(f_text)")    
    if fExcecute then
        sCheckValue("116")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tLENGTH()
    
    sInit(gFileName,"TT_Func1")
    sSet("LENGTH(f_text)")       
    if fExcecute then
        sCheckValue("4") 
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tOCTET_LENGTH()
    
    sInit(gFileName,"TT_Func1")
    sSet("OCTET_LENGTH(f_text)")        
    if fExcecute then
        sCheckValue("4")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tCHAR_LENGTH()
    
    sInit(gFileName,"TT_Func1")
    sSet("CHAR_LENGTH(f_text)")    
    if fExcecute then
        sCheckValue("4")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tCHARACTER_LENGTH()
    
    sInit(gFileName,"TT_Func1")
    sSet("CHARACTER_LENGTH(f_text)")    
    if fExcecute then
        sCheckValue("4")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tCHAR()
    
    sInit(gFileName,"TT_Func1")
    sSet("CHAR(120,121)") ' xy
    if fExcecute then    
        sCheckValue("xy")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tCONCAT()
    
    sInit(gFileName,"TT_Func1")
    sSet("CONCAT(f_text,f_text)")    
    if fExcecute then
        sCheckValue("texttext")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tLOCATE()
    
    sInit(gFileName,"TT_Func1")
    sSet("LOCATE('ex',f_text)")    
    if fExcecute then
        sCheckValue("2")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tSUBSTRING1()
    
    sInit(gFileName,"TT_Func1")
    sSet("SUBSTRING(f_text,2)")    
    if fExcecute then
        sCheckValue("ext")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tSUBSTRING2()
    
    sInit(gFileName,"TT_Func1")
    sSet("SUBSTRING(f_text,2,2)")    
    if fExcecute then
        sCheckValue("ex")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tSUBSTRING3()
    
    sInit(gFileName,"TT_Func1")
    sSet("SUBSTRING(f_text FROM 2 FOR 2)")    
    if fExcecute then
        sCheckValue("ex")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tLTRIM()
    
    sInit(gFileName,"TT_Func1")
    sSet("LTRIM('   text ')")    
    if fExcecute then
        sCheckValue("text ")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tRTRIM()
    
    sInit(gFileName,"TT_Func1")
    sSet("RTRIM(' text  ')")    
    if fExcecute then
        sCheckValue(" text")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tSPACE()
    
    sInit(gFileName,"TT_Func1")
    sSet("SPACE(5)")    
    if fExcecute then
        sCheckValue("     ")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tREPLACE()
    
    sInit(gFileName,"TT_Func1")
    sSet("REPLACE(f_text,'ex','xe')")    
    if fExcecute then
        sCheckValue("txet")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tREPEAT()
    
    sInit(gFileName,"TT_Func1")
    sSet("REPEAT(f_text,5)")    
    if fExcecute then
        sCheckValue("texttexttexttexttext")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tINSERT()
    
    sInit(gFileName,"TT_Func1")
    sSet("INSERT(f_text,2,2,'tt')")    
    if fExcecute then
        sCheckValue("tttt")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tLEFT()
    
    sInit(gFileName,"TT_Func1")
    sSet("LEFT(f_text,2)")    
    if fExcecute then
        sCheckValue("te")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tRIGHT()
    
    sInit(gFileName,"TT_Func1")
    sSet("RIGHT(f_text,2)")    
    if fExcecute then
        sCheckValue("xt")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------        
'Numeric fucntions
'-------------------------------------------------------------------------       
testcase tABS()
    
    sInit(gFileName,"TT_Func1")
    sSet("ABS('1,6')")    
    if fExcecute then
        sCheckValue("1")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tSIGN()
    
    sInit(gFileName,"TT_Func1")
    sSet("SIGN('-2')")    
    if fExcecute then
        sCheckValue("-1")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tMOD()
    
    sInit(gFileName,"TT_Func1")
    sSet("MOD(1,0)")    
    if fExcecute then
        'sCheckValue("1.#NAN") ' this doens't work because sometimes the result is 
        '1.#NAN and sometimes it is -1.#NAN. This have to be investigate in the future.
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tFLOOR()
    
    sInit(gFileName,"TT_Func1")
    sSet("FLOOR(1.789)")    
    if fExcecute then
        sCheckValue("1")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tCEILING()
    
    sInit(gFileName,"TT_Func1")
    sSet("CEILING(1.789)")    
    if fExcecute then
        sCheckValue("2")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tROUND1()
    
    sInit(gFileName,"TT_Func1")
    sSet("ROUND(1.789)")    
    if fExcecute then
        sCheckValue("2")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tROUND2()
    
    sInit(gFileName,"TT_Func1")
    sSet("ROUND(1.789,1)")    
    if fExcecute then
        sCheckValue("1.8")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tEXP()
    
    sInit(gFileName,"TT_Func1")
    sSet("EXP(1)")    
    if fExcecute then
        sCheckValue("2.72")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tLN()
     
    sInit(gFileName,"TT_Func1")
    sSet("LN(1)")    
    if fExcecute then
        sCheckValue("0")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tLOG1()
    
    sInit(gFileName,"TT_Func1")
    sSet("LOG(1)")    
    if fExcecute then
        sCheckValue("0")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tLOG2()
    
    sInit(gFileName,"TT_Func1")
    sSet("LOG(2,3)")    
    if fExcecute then
        sCheckValue("0.63")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tLOG10()
    
    sInit(gFileName,"TT_Func1")
    sSet("LOG10(1)")    
    if fExcecute then
        sCheckValue("0")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tPOWER()
    
    sInit(gFileName,"TT_Func1")
    sSet("POWER(2,3)")    
    if fExcecute then
        sCheckValue("8")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tSQRT()
    
    sInit(gFileName,"TT_Func1")
    sSet("SQRT(2)")    
    if fExcecute then
        sCheckValue("1.41")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------       
testcase tPI()
    
    sInit(gFileName,"TT_Func1")
    sSet("PI()")    
    if fExcecute then
        sCheckValue("3.14")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tCOS()
    
    sInit(gFileName,"TT_Func1")
    sSet("COS(2)")    
    if fExcecute then
        sCheckValue("-0.42")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tSIN()
    
    sInit(gFileName,"TT_Func1")
    sSet("SIN(2)")    
    if fExcecute then
        sCheckValue("0.91")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tTAN()
    
    sInit(gFileName,"TT_Func1")
    sSet("TAN(2)")    
    if fExcecute then
        sCheckValue("-2.19")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tACOS()
    
    sInit(gFileName,"TT_Func1")
    sSet("ACOS(0.2)")    
    if fExcecute then
        sCheckValue("1.37")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tASIN()
    
    sInit(gFileName,"TT_Func1")
    sSet("ASIN(0.2)")    
    if fExcecute then
        sCheckValue("0.2")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tATAN()
    
    sInit(gFileName,"TT_Func1")
    sSet("ATAN(2)")    
    if fExcecute then
        sCheckValue("1.11")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tATAN2()
    
    sInit(gFileName,"TT_Func1")
    sSet("ATAN2(2,2)")    
    if fExcecute then
        sCheckValue("0.79")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tDEGREES()
    
    sInit(gFileName,"TT_Func1")
    sSet("DEGREES(2)")    
    if fExcecute then
        sCheckValue("114.59")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tRADIANS()
    
    sInit(gFileName,"TT_Func1")
    sSet("RADIANS(2)")    
    if fExcecute then
        sCheckValue("0.03")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------        
'Date / Time fucntions
'-------------------------------------------------------------------------
testcase tDAYOFWEEK()
    
    sInit(gFileName,"TT_Func1")
    sSet("DAYOFWEEK('2003-10-13')")    
    if fExcecute then
        sCheckValue("2")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tDAYOFMONTH()
    
    sInit(gFileName,"TT_Func1")
    sSet("DAYOFMONTH('2003-10-13')")    
    if fExcecute then
        sCheckValue("13")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tDAYOFYEAR()
    
    sInit(gFileName,"TT_Func1")
    sSet("DAYOFYEAR('2003-10-13')")    
    if fExcecute then
        sCheckValue("286")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tMONTH()
    
    sInit(gFileName,"TT_Func1")
    sSet("MONTH('2003-10-13')")    
    if fExcecute then
        sCheckValue("10")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tDAYNAME()
    
    sInit(gFileName,"TT_Func1")
    sSet("DAYNAME('2003-10-13')")    
    if fExcecute then
        sCheckValue("Monday")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tMONTHNAME()
    
    sInit(gFileName,"TT_Func1")
    sSet("MONTHNAME('2003-10-13')")    
    if fExcecute then
        sCheckValue("October")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tQUARTER()
    
    sInit(gFileName,"TT_Func1")
    sSet("QUARTER('2003-10-13')")    
    if fExcecute then
        sCheckValue("4")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tWEEK()
    
    sInit(gFileName,"TT_Func1")
    sSet("WEEK('2003-10-13')")    
    if fExcecute then
        sCheckValue("42")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tWEEK2()
    
    sInit(gFileName,"TT_Func1")
    sSet("WEEK('2003-10-13',1)")    
    if fExcecute then
        sCheckValue("41")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tYEAR()
    
    sInit(gFileName,"TT_Func1")
    sSet("YEAR('2003-10-13')")    
    if fExcecute then
        sCheckValue("2003")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tHOUR()
    
    sInit(gFileName,"TT_Func1")
    sSet("HOUR('12:14:25')")    
    if fExcecute then
        sCheckValue("12")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tMINUTE()
    
    sInit(gFileName,"TT_Func1")
    sSet("MINUTE('12:14:25')")    
    if fExcecute then
        sCheckValue("14")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tSECOND()
    
    sInit(gFileName,"TT_Func1")
    sSet("SECOND('12:14:25')")    
    if fExcecute then
        sCheckValue("25")
    endif
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tCURDATE()
    
    sInit(gFileName,"TT_Func1")
    sSet("CURDATE()")    
    fExcecute 
    'the value can't be checked 
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tCURTIME()
    
    sInit(gFileName,"TT_Func1")
    sSet("CURTIME()")    
    fExcecute
    'the value can't be checked
    sDeInit()

endcase
'-------------------------------------------------------------------------
testcase tNOW()
    
    sInit(gFileName,"TT_Func1")
    sSet("NOW()")    
    fExcecute
    'the value can't be checked        
    sDeInit()

endcase
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
'-----------P R I V A T E    F U N C T I O N S----------------------------
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
sub sSet( sString )
    Kontext "QueryDesignCriterion"                           
        '/// set value for the field
        printlog "set " + sString  + " for the field"
        Field.setText(sString)
        sleep(1)
end sub          
'-------------------------------------------------------------------------
function fExcecute

    Dim bOK as boolean    
    bOK = true
    Kontext "Toolbar"
        '/// Execute the query
        printlog "Execute the query"
        ExecuteBtn.Click
        sleep(3)
    Kontext "MessageBox"
        if MessageBox.exists(3) then
            warnlog "error message when executing query"
            MessageBox.OK
            bOK = false
        endif        
    Kontext "TableView"
        if NOT AllRecords.Exists(3) then
            warnlog "Execution of a query failed!"
            bOK = false
        endif
    fExcecute = bOK
end function
'-------------------------------------------------------------------------
sub sInit( gFileName, sTableName )
    
    call fOpenDatabase(gFileName)
    
    call fOpenNewQueryDesign

    call fChooseTableInAddTableDialog(sTableName)
    
end sub                
'-------------------------------------------------------------------------               
sub sDeInit

    call fCloseQueryDesign
    call fCloseDatabase
    
end sub
'-------------------------------------------------------------------------
sub sCheckValue(sValue)
    Dim sClipboard as string
    Dim i as integer
    '/// check if the value is right
    printlog "check if the value is " + sValue
    Kontext "TableView"        
        if AllRecords.Exists(3) then
            Kontext "TableView"                
                TableView.TypeKeys "<HOME>" , true
                wait(100)
                TableView.TypeKeys "<SHIFT END>" , true
                wait(100)
                TableView.TypeKeys "<MOD1 C>" , true
                wait(100)
                sClipboard = GetClipboard
                i = instr(sClipboard,",")
                if i <> 0 then
                    sClipboard = Left(sClipboard,i -1) + "." + Right(sClipboard,len(sClipboard) - i) 
                end if
                if StrComp (sValue, sClipboard , 1) <> 0 then 
                    warnlog "it should " + sValue + " it is " + GetClipboard
                endif
        else
            warnlog "Execution of a query failed!"
        end if  
end sub

