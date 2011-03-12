' Sample-Programm fuer Sample-Objekte

Sub Main
	Dim Sample As SampleObject
	Dim Element1 As Object, Element2 As Object
	Set Element1 = Sample!Create "Objekt"
	Set Element2 = Sample.Create "Objekt"
	Element1 = "Element 1"
	Element2 = "Element 2"
	For i = 0 to 1
		Print Sample.Objekt( i )
	Next
	'Test der Event-Methode im Sample-Objekt
	Sample.Event "Bang"
End Sub

Sub Bang
	print "Sample-Callback: BANG!"
End Sub




















