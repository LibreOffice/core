REM  *****  BASIC  *****


Sub Main
factory= createUnoService("com.sun.star.bridge.OleObjectFactory")
obj= factory.createInstance("AxTestComponents.Basic")


obj.prpObject= obj
prpObject= obj.prpObject
'identity of objects, mapped from COM to UNO, is not given currently 
if NOT equalUnoObjects(obj,prpObject) then 
	MsgBox "error"
end if

'properties -------------------------------------------------------------------------
Dim ar1(1)
ar1(0)= "1"
ar1(1)= "2"
obj.prpArray= ar1()
prpArray= obj.prpArray
if (prpArray(0) <> "1") OR (prpArray(1) <> "2") then 
	MsgBox "error"
end if

obj.prpVariant="string"
prpVariant= obj.prpVariant
if prpVariant <> "string" then
	MsgBox "error"
end if

obj.prpDouble=3.145
prpDouble= obj.prpDouble
if NOT ((prpDouble >3.144) AND (prpDouble < 3.146)) then
	MsgBox "error"
end if


obj.prpFloat= 3.14
prpFloat= obj.prpFloat
if NOT ((prpFloat >3.13) AND (prpFloat < 3.15)) then
	MsgBox "error"
end if

obj.prpString= "string"
prpString= obj.prpString
if prpString <> "string" then
	MsgBox "error"
end if

obj.prpLong= 1000
prpLong= obj.prpLong
if prpLong <> 1000 then
	MsgBox "error"
end if

obj.prpShort= 127
prpShort= obj.prpShort
if prpShort <> 127 then
	MsgBox "error"
end if

obj.prpByte= 11
prpByte= obj.prpByte
if prpByte <> 11 then 
	MsgBox "error"
end if


'out parameter -------------------------------------------------------------------------
Dim outObject
obj.outObject(outObject)
if outObject.prpString <> "out" then 
	MsgBox "error"
end if

Dim outArray
obj.outArray(outArray)
if (outArray(0) <> "out1") OR (outArray(1) <> "out2") OR (outArray(2) <> "out3")then
	MsgBox "error"
end if
Dim outVariant
obj.outVariant(outVariant)
if outVariant <> "out" then
	MsgBox "error"
end if 
Dim outDouble
obj.outDouble(outDouble)
if NOT ((outDouble >3.144) AND (outDouble < 3.146)) then
	MsgBox "error"
end if


Dim outFloat
obj.outFloat(outFloat)
if NOT ((outFloat >3.13) AND (outFloat < 3.15)) then
	MsgBox "error"
end if

Dim outString
obj.outString(outString)
if outString <> "out" then
	MsgBox "error"
end if
Dim outLong
obj.outLong(outLong)
if outLong <> 111111 then
	MsgBox "error"
end if

Dim outShort
obj.outShort(outShort)
if outShort <> 1111 then
	MsgBox "error"
end if

Dim outByte
obj.outByte(outByte)
if outByte <> 111 then
	MsgBox "error"
end if


'in-out parameter -------------------------------------------------------------
obj2= factory.createInstance("AxTestComponents.Basic")
inoutObj= obj2
inoutObj.prpString="in"
obj.inoutObject(inoutObj)
if equalUnoObjects(inoutObj,obj) then
	MsgBox "error"
end if 

if inoutObj.prpString <> "out" then
	MsgBox "error"
end if


Dim inoutAr(2)
inoutAr(0)="1"
inoutAr(1)="2"
inoutAr(2)="3"
obj.inoutArray(inoutAr())

if (inoutAr(0) <> "1out") OR (inoutAr(1) <> "2out") OR (inoutAr(2) <> "3out") then
	MsgBox "error"
end if

inoutVar= "in"
obj.inoutVariant(inoutVar)
if inoutVar <> "inout" then
	MsgBox "error"
end if 

inoutDouble= 3.14
obj.inoutDouble(inoutDouble)
if NOT ((inoutDouble >4.13) AND (inoutDouble < 4.15)) then
	MsgBox "error"
end if

inoutFloat= 3.14
obj.inoutFloat(inoutFloat)
if NOT ((inoutFloat >4.13) AND (inoutFloat < 4.15)) then
	MsgBox "error"
end if

inoutString= "in"
obj.inoutString(inoutString)
if inoutString <> "inout" then
	MsgBox "error"
end if 


inoutLong= 10
obj.inoutLong(inoutLong)
if inoutLong <> 11  then 
 MsgBox "error"
end if 

inoutShort= 10
obj.inoutShort(inoutShort)
if inoutShort <> 11  then 
 MsgBox "error"
end if 


inoutByte= 10
obj.inoutByte(inoutByte)
if inoutByte <> 11  then 
 MsgBox "error"
end if 


'in parameter -------------------------------------------------------------------
obj.inByte(11)
obj.inShort(111)
obj.inLong(11111)
obj.inString("I am a string")
obj.inFloat(3.14)
obj.inDouble(3.145)
obj.inVariant("I am a string in a variant")
obj.prpString= "a string property"
obj.inObject(obj)

Dim arString(1) as String
arString(0)= "String one"
arString(1)= "String two"
obj.inArray(arString())

End Sub
