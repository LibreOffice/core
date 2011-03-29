Sub Main
		' Oeffnen der LOG-Datei
		' Es gibt 2 Moeglichgkeiten, diesen Test zu absolvieren.
		' 1) Ausgabe von Informationen in MessageBoxen
		' 2) Ausgabe von Informationen in einer LOG-Datei
		'
		' Die Methoden OpenLOG, CloseLOG, Message, ErrorMessage und InfoMessage beruecksichtigen das automatisch!
		' Zum Umschalten zwischen den beiden Zustaenden genuegt es, eine der folgenden
		' zwei Programmzeilen zu aktivieren ...

		'bLOGOn	= cOn
		bLOGOn	= cOff

		'bShowErrorsOnly		= cOn
		bShowErrorsOnly		= cOff

		OpenLOG		( cTestPath + "TestDocumentProperties.log" )
		InfoMessage	( "Test DocumentProperties ... [start]" )

		' Service besorgen
		InfoMessage ( "Service besorgen ... [start]" )
		aDocumentProperties = createUnoService ( "com.sun.star.document.DocumentProperties" )

		' Erfolg abtesten
		if ( isnull ( aDocumentProperties ) ) then
			ErrorMessage ( "Service konnte nicht instanziiert werden!"	)
			exit sub
		else
			InfoMessage ( "Service erfolgreich instanziiert ..."	)
			InfoMessage ( "Service besorgen ... [ende]"				)
		end if

		' Unterstuetzte Schnittstellen, Methoden und Properties anzeigen
		' Achtung: Methoden und Properties koennen nicht angezeigt werden ...
		' neues Uno <-> Basic !?
		msgbox	aDocumentProperties.dbg_supportedInterfaces
		'msgbox	aDocumentProperties.dbg_methods
		'msgbox	aDocumentProperties.dbg_properties

		' Testen des Services unter normalen Bedingungen (also wie vorgesehen)
		bState = Test_NormalUse	( aDocumentProperties )
		' Fehlerstatus abfragen
		if ( bState = cError ) then
			ErrorMessage ( "Der Service arbeitet unter normalen Bedingungen nicht korrekt!" )
		end if

		bState = Test_ErrorUse	( aDocumentProperties )
		' Fehlerstatus abfragen
		if ( bState = cError ) then
			ErrorMessage ( "Der Service verhaelt sich in Fehlersituationen nicht korrekt!" )
		end if

		' Schliessen der Error-Logdatei
		InfoMessage ( "Test DocumentProperties ... [ende]" )
		CloseLOG
End Sub

'*****************************************************************************************************************
' Testfunktionen
'*****************************************************************************************************************

'----------------------------------------------------------------------------
' Testmethode: Testet den Service unter normalen Bedingungen
'
' Returnwert cOK bedeutet, dass sich das Objekt normal verhaelt ...
' Returnwert cError bedeutet, dass sich das ein Fehler aufgetreten ist ...
'----------------------------------------------------------------------------
Function Test_NormalUse ( aDocumentProperties ) as Boolean

		' Zunaechst wird vom Erfolg des Test ausgegangen.
		' Sollte einer der Detail-Tests fehlschlagen, dann wird dieser Wert
		' zurueckgesetzt. Damit wird dann angezeigt, dass mindestens ein
		' Einzeltest nicht korrekt funktionierte.

		Test_NormalUse = cOK

		bState = Test_NormalUse_XPropertySet	( aDocumentProperties )
		if ( bState = cError ) then
			Test_NormalUse = cError
		end if

		bState = Test_NormalUse_XNameContainer	( aDocumentProperties )
		if ( bState = cError ) then
			Test_NormalUse = cError
		end if

		bState = Test_NormalUse_XPersist		( aDocumentProperties )
		if ( bState = cError ) then
			Test_NormalUse = cError
		end if

End Function

'----------------------------------------------------------------------------
' Testmethode: Testet den Service unter Randbedingungen und provoziert Fehlerzustaende
'
' Returnwert cOK bedeutet, dass das Objekt damit keine Probleme hat ...
' Returnwert cError bedeutet, dass das Objekt noch nicht robust genug ist ...
'----------------------------------------------------------------------------
Function Test_ErrorUse ( aDocumentProperties ) as Boolean

		Test_ErrorUse = cOK

End Function

'----------------------------------------------------------------------------
' Testmethode: Testen des unterstuetzten Interfaces "XPropertySet" unter normalen Bedingungen
'
' Returnwert = cOK		; wenn der Test erfolgreich war
' Returnwert = cError	; wenn sich das Objekt nicht korrekt verhalten hat
'----------------------------------------------------------------------------
Function Test_NormalUse_XPropertySet ( aDocumentProperties ) as Boolean

		' Einschalten der Fehlerbehandlung, um Exceptions oder aehnliches abzufangen!
		on Error goto Test_NormalUse_XPropertySet_Error

		InfoMessage ( "Test_NormalUse_XPropertySet ... [start]" )

		' 1) Test der Funktion "getPropertySetInfo()"
		'		Da diese Funktion keine Parameter besitzt und zudem eine "get"-Methode
		'		darstellt, wird sie durch Basic automatisch als "Property" behandelt!
		'		Daher schreibt man nicht "getPropertySetInfo()" sondern nur "PropertySetInfo".

		' Besorgen der Info
		PropertySetInfo = aDocumentProperties.PropertySetInfo
		' Und abtesten auf Gueltigkeit
		if ( isnull (PropertySetInfo) ) then
			ErrorMessage ( "getPropertySetInfo() ... Error (Keine Info bestimmbar!)" )
			goto Test_NormalUse_XPropertySet_Error
		end if

		' Hier fehlt noch der Test der InfoStruktur! (Laesst sich unter Basic irgendwie NICHT testen!!!???)
		' ...

		InfoMessage ( "getPropertySetInfo() ... OK" )

		' 2) getPropertyValue() & setPropertyValue ()
		'		In diesem Service sind mehrere Properties bereits definiert und vorbelegt.
		'		Zum Test werden repraesentativ einige davon verwendet. Naemlich je eine
		'		der verschiedenen Datentypen!
		'		Das sind im folgenden:		OWString, sal_Bool, sal_Int16, sal_uInt16, sal_Int32, DateTime, Sequence< sal_Int8 >
		'									Achtung! sal_uInt16 kann in Basic so nicht dargestellt werden. Daher wird ein normaler
		'									Integer-Wert angenommen - Bedingung ist, das hier im Test der Wertebereich nicht
		'									ueberschritten wird!
		'		Es wird versucht den Standardwert dieser zu ermitteln und zu merken;
		'		dann einen neuen Wert zu setzen; sowie diesen wiederum zu lesen und mit den
		'		vorherigen Werten zu vergleichen!

		' Zunaechst werden die Standardwerte dieser Properties besorgt ...
		sDefaultValue_OWString$		= aDocumentProperties.getPropertyValue ( "Author"			)
		bDefaultValue_sal_Bool		= aDocumentProperties.getPropertyValue ( "AutoloadEnabled"	)
		nDefaultValue_sal_Int16%	= aDocumentProperties.getPropertyValue ( "EditingCycles"	)
		nDefaultValue_sal_uInt16%	= aDocumentProperties.getPropertyValue ( "Priority"			)
		nDefaultValue_sal_Int32&	= aDocumentProperties.getPropertyValue ( "EditingDuration"	)
		aDefaultValue_DateTime		= aDocumentProperties.getPropertyValue ( "ModifyDate"		)
'		aDefaultValue_ByteSequence	= aDocumentProperties.getPropertyValue ( "ExtraData"		)
		aDefaultValue_ByteSequence	= aDocumentProperties.ExtraData

		' ... dann die Werte zur Kontrolle ausgeben.
		ShowProperties ( aDocumentProperties )

		InfoMessage ( "getPropertyValue() ... OK" )

		' Jetzt werden neue Werte vereinbart ...
		' Diese werden so gewaehlt, das sie garantiert von den Standardwerten verschieden sind!
		' Dazu werden die alten auf Wert abgefragt und entsprechend die neuen gesetzt.
		sNewValue_OWString$		= sDefaultValue_OWString$	+ "NeuerWert"
		bNewValue_sal_Bool		= not bDefaultValue_sal_Bool
		nNewValue_sal_Int16%	= nDefaultValue_sal_Int16%	+ 1
		if ( nDefaultValue_sal_uInt16% = 1 ) then
			nNewValue_sal_uInt16% = 2
		else
			nNewValue_sal_uInt16% = 1
		end if
		nNewValue_sal_Int32&	= nDefaultValue_sal_Int32&  + 1

		aNewValue_DateTime					= aDefaultValue_DateTime
		aNewValue_DateTime.HundredthSeconds	= aDefaultValue_DateTime.HundredthSeconds	+ 1
		aNewValue_DateTime.Seconds			= aDefaultValue_DateTime.Seconds         	+ 1
		aNewValue_DateTime.Minutes			= aDefaultValue_DateTime.Minutes         	+ 1
		aNewValue_DateTime.Hours			= aDefaultValue_DateTime.Hours           	+ 1
		aNewValue_DateTime.Day				= aDefaultValue_DateTime.Day             	+ 1
		aNewValue_DateTime.Month			= aDefaultValue_DateTime.Month           	+ 1
		aNewValue_DateTime.Year				= aDefaultValue_DateTime.Year            	+ 1

		aNewValue_ByteSequence = aDefaultValue_ByteSequence
		nElementCount% = UBound ( aDefaultValue_ByteSequence )
'		for nCounter%=0 to nElementCount% step 1
'			aNewValue_ByteSequence(nCounter%) = ( aDefaultValue_ByteSequence(nCounter%) + 1 )
'		next nCounter%

		' Anschliessend muessen diese neuen Werte gesetzt werden.
		aDocumentProperties.setPropertyValue ( "Author"			, sNewValue_OWString$		)
		aDocumentProperties.setPropertyValue ( "AutoloadEnabled", bNewValue_sal_Bool		)
		aDocumentProperties.setPropertyValue ( "EditingCycles"	, nNewValue_sal_Int16%		)
		aDocumentProperties.setPropertyValue ( "Priority"		, nNewValue_sal_uInt16%		)
		aDocumentProperties.setPropertyValue ( "EditingDuration", nNewValue_sal_Int32&		)
		aDocumentProperties.setPropertyValue ( "ModifyDate"		, aNewValue_DateTime		)
'		aDocumentProperties.setPropertyValue ( "ExtraData"		, aNewValue_ByteSequence	)
		aDocumentProperties.ExtraData = aNewValue_ByteSequence

		' Dann lassen wir sie uns ausgeben, um sie mit den vorherigen vergleichen zu koennen.
		' (Das geht natuerlich nur, wenn "bLOGOn=cOn" ist - also eine LOG-Datei geschrieben wird!)
		ShowProperties ( aDocumentProperties )

		' Nun werden die Properties wieder gelesen ...
		sLastValue_OWString$	= aDocumentProperties.getPropertyValue ( "Author"			)
		bLastValue_sal_Bool		= aDocumentProperties.getPropertyValue ( "AutoloadEnabled"	)
		nLastValue_sal_Int16%	= aDocumentProperties.getPropertyValue ( "EditingCycles"	)
		nLastValue_sal_uInt16%	= aDocumentProperties.getPropertyValue ( "Priority"			)
		nLastValue_sal_Int32&	= aDocumentProperties.getPropertyValue ( "EditingDuration"	)
		aLastValue_DateTime		= aDocumentProperties.getPropertyValue ( "ModifyDate"		)
		aLastValue_ByteSequence	= aDocumentProperties.getPropertyValue ( "ExtraData"		)

		' ... und mit den vorher als zu setzend bestimmte Werte vergleichen!
		' Es duerfen KEINE Unterschiede auftreten, da sonst "setPropertyValue()" nicht korrekt funktioniert hat!

		if ( CompareOWString			( sNewValue_OWString$,		sLastValue_OWString$	) = cDifferent ) then
			ErrorMessage	( "setPropertyValue() ... Fehler [OWString fehlerhaft]"		)
			goto Test_NormalUse_XPropertySet_Error
		elseif ( CompareBool			( bNewValue_sal_Bool,		bLastValue_sal_Bool		) = cDifferent ) then
			ErrorMessage	( "setPropertyValue() ... Fehler [sal_Bool fehlerhaft]"		)
			goto Test_NormalUse_XPropertySet_Error
		elseif ( CompareInt16			( nNewValue_sal_Int16%,		nLastValue_sal_Int16%	) = cDifferent ) then
			ErrorMessage	( "setPropertyValue() ... Fehler [sal_Int16 fehlerhaft]"	)
			goto Test_NormalUse_XPropertySet_Error
		elseif ( CompareInt16			( nNewValue_sal_uInt16%,	nLastValue_sal_uInt16%	) = cDifferent ) then
			ErrorMessage	( "setPropertyValue() ... Fehler [sal_uInt16 fehlerhaft]"	)
			goto Test_NormalUse_XPropertySet_Error
		elseif ( CompareInt32			( nNewValue_sal_Int32&,		nLastValue_sal_Int32&	) = cDifferent ) then
			ErrorMessage	( "setPropertyValue() ... Fehler [sal_Int32 fehlerhaft]"	)
			goto Test_NormalUse_XPropertySet_Error
		elseif ( CompareDateTime		( aNewValue_DateTime,		aLastValue_DateTime		) = cDifferent ) then
			ErrorMessage	( "setPropertyValue() ... Fehler [DateTime fehlerhaft]"		)
			goto Test_NormalUse_XPropertySet_Error
		elseif ( CompareByteSequence	( aNewValue_ByteSequence,	aLastValue_ByteSequence	) = cDifferent ) then
			ErrorMessage	( "setPropertyValue() ... Fehler [ByteSequence fehlerhaft]"	)
			goto Test_NormalUse_XPropertySet_Error
		end if

		InfoMessage ( "setPropertyValue() ... OK" )

		' Nun wird noch mit den zuerst ermittelten Default-Werten verglichen!
		' Hier MUESSEN Unterschiede auftreten, da sonst "get-" UND "setPropertyValue()" nicht korrekt funktioniert haben!

		if ( CompareOWString			( sDefaultValue_OWString$,		sLastValue_OWString$	) = cEqual ) then
			ErrorMessage	( "Zusammenspiel set & getPropertyValue() ... Fehler [OWString fehlerhaft]"		)
			goto Test_NormalUse_XPropertySet_Error
		elseif ( CompareBool			( bDefaultValue_sal_Bool,		bLastValue_sal_Bool		) = cEqual ) then
			ErrorMessage	( "Zusammenspiel set & getPropertyValue() ... Fehler [sal_Bool fehlerhaft]"		)
			goto Test_NormalUse_XPropertySet_Error
		elseif ( CompareInt16			( nDefaultValue_sal_Int16%,		nLastValue_sal_Int16%	) = cEqual ) then
			ErrorMessage	( "Zusammenspiel set & getPropertyValue() ... Fehler [sal_Int16 fehlerhaft]"	)
			goto Test_NormalUse_XPropertySet_Error
		elseif ( CompareInt16			( nDefaultValue_sal_uInt16%,	nLastValue_sal_uInt16%	) = cEqual ) then
			ErrorMessage	( "Zusammenspiel set & getPropertyValue() ... Fehler [sal_uInt16 fehlerhaft]"	)
			goto Test_NormalUse_XPropertySet_Error
		elseif ( CompareInt32			( nDefaultValue_sal_Int32&,		nLastValue_sal_Int32&	) = cEqual ) then
			ErrorMessage	( "Zusammenspiel set & getPropertyValue() ... Fehler [sal_Int32 fehlerhaft]"	)
			goto Test_NormalUse_XPropertySet_Error
		elseif ( CompareDateTime		( aDefaultValue_DateTime,		aLastValue_DateTime		) = cEqual ) then
			ErrorMessage	( "Zusammenspiel set & getPropertyValue() ... Fehler [DateTime fehlerhaft]"		)
			goto Test_NormalUse_XPropertySet_Error
'		elseif ( CompareByteSequence	( aDefaultValue_ByteSequence,	aLastValue_ByteSequence	) = cEqual ) then
'			ErrorMessage	( "Zusammenspiel set & getPropertyValue() ... Fehler [ByteSequence fehlerhaft]"	)
'			goto Test_NormalUse_XPropertySet_Error
		end if

		InfoMessage ( "Zusammenspiel set & getPropertyValue() ... OK" )

		' Der Test war erfolgreich! Meldung ausgeben und zurueck zm Aufrufer.
		' Ausschalten der Fehlerbehandlung
		on Error goto 0
		' Meldung ausgeben
		InfoMessage ( "Test_NormalUse_XPropertySet ... [ende]" )
		' Status setzen
		Test_NormalUse_XPropertySet = cOK
		' Und Funktion beenden
		Exit Function

' Es ist ein unerwartete Fehler aufgetreten! (Exception ...)
' Meldung ausgeben und mit Fehler zurueckkehren.
Test_NormalUse_XPropertySet_Error:
		' Ausschalten der Fehlerbehandlung
		on Error goto 0
		' Meldung ausgeben
		ErrorMessage ( "Test_NormalUse_XPropertySet ... [Error]" )
		' und Fehlerstatus setzen
		Test_NormalUse_XPropertySet = cError
		' Abbruch der Funktion erzwingen!
		Exit Function

End Function

'----------------------------------------------------------------------------
' Testmethode: Testen des unterstuetzten Interfaces "XNameContainer" unter normalen Bedingungen
'
' Returnwert = cOK		; wenn sich das Objekt korrekt verhalten hat
' Returnwert = cError	; wenn das Objekt noch nicht robust genug ist
'----------------------------------------------------------------------------
Function Test_NormalUse_XNameContainer ( aDocumentProperties ) as Boolean

		' Einschalten der Fehlerbehandlung, um Exceptions oder aehnliches abzufangen!
		on Error goto Test_NormalUse_XNameContainer_Error

		InfoMessage ( "Test_NormalUse_XNameContainer ... [start]" )

		' Da das Initialisieren im Konstruktor des Objektes und das Aufraeumen im Destruktor
		' automatisch geschieht und diese Methode pro Programmablauf nur einmal verwendet wird,
		' darf sich kein Element schon im NameContainer befinden!
		' Wenn doch, ist das ein Fehler!
		if ( aDocumentProperties.hasElements () = TRUE ) then
			ErrorMessage ( "Der NameConatiner sollte eigentlich leer sein, enthaelt initial aber schon Elemente!?" )
			goto Test_NormalUse_XNameContainer_Error
		end if

		' Zunaechst werden mehrere Elemente in den NameContainer eingefuegt.
		sItemName_1$	= "Item 1"
		sItemName_2$	= "Item 2"
		sItemName_3$	= "Item 3"
		sItemName_4$	= "Item 4"
		sItemName_5$	= "Item 5"

		sFirstValue_1$	= "Value 1"
		sFirstValue_2$	= "Value 2"
		sFirstValue_3$	= "Value 3"
		sFirstValue_4$	= "Value 4"
		sFirstValue_5$	= "Value 5"

		aDocumentProperties.insertByName ( sItemName_1$, sFirstValue_1$ )
		aDocumentProperties.insertByName ( sItemName_2$, sFirstValue_2$ )
		aDocumentProperties.insertByName ( sItemName_3$, sFirstValue_3$ )
		aDocumentProperties.insertByName ( sItemName_4$, sFirstValue_4$ )
		aDocumentProperties.insertByName ( sItemName_5$, sFirstValue_5$ )

		' Zur Kontrolle die Werte ausgeben. (Nur wichtig, wenn geloggt wird!)
		' Dabei wird die Methode "getElementNames()" gleich implizit mitgetestet!
		ShowNameContainer ( aDocumentProperties )

		' Aber auch die Anzahl kontrollieren.
		' (Hier wird eine eigene BASIC-Hilfsfunktion verwendet! keine Interface-Methode)
		if ( getNameContainerCount ( aDocumentProperties ) <> 5 ) then
			ErrorMessage ( "insertByName() ... Fehler (Der NameConatiner enthaelt nicht die eingefuegten 5 Elemente!)" )
			goto Test_NormalUse_XNameContainer_Error
		end if

		' Nun noch feststellen, ob die 5 denn auch tatsaechlich vorhanden sind.
		if ( aDocumentProperties.hasByName ( sItemName_1$ ) = FALSE ) then
			ErrorMessage	( "hasByName() ... Fehler [Element 1 nicht vorhanden!?]" )
			goto Test_NormalUse_XNameContainer_Error
		elseif ( aDocumentProperties.hasByName ( sItemName_2$ ) = FALSE ) then
			ErrorMessage	( "hasByName() ... Fehler [Element 2 nicht vorhanden!?]" )
			goto Test_NormalUse_XNameContainer_Error
		elseif ( aDocumentProperties.hasByName ( sItemName_3$ ) = FALSE ) then
			ErrorMessage	( "hasByName() ... Fehler [Element 3 nicht vorhanden!?]" )
			goto Test_NormalUse_XNameContainer_Error
		elseif ( aDocumentProperties.hasByName ( sItemName_4$ ) = FALSE ) then
			ErrorMessage	( "hasByName() ... Fehler [Element 4 nicht vorhanden!?]" )
			goto Test_NormalUse_XNameContainer_Error
		elseif ( aDocumentProperties.hasByName ( sItemName_5$ ) = FALSE ) then
			ErrorMessage	( "hasByName() ... Fehler [Element 5 nicht vorhanden!?]" )
			goto Test_NormalUse_XNameContainer_Error
		end if

		' Dann die Werte wieder auslesen.
		sCompareValue_1$ = aDocumentProperties.getByName ( sItemName_1$ )
		sCompareValue_2$ = aDocumentProperties.getByName ( sItemName_2$ )
		sCompareValue_3$ = aDocumentProperties.getByName ( sItemName_3$ )
		sCompareValue_4$ = aDocumentProperties.getByName ( sItemName_4$ )
		sCompareValue_5$ = aDocumentProperties.getByName ( sItemName_5$ )

		' Zum Vergleich die Werte ausgeben. (Nur wichtig, wenn geloggt wird!)
		ShowNameContainer ( aDocumentProperties )

		' Dann die Werte automatisch vergleichen!
		if ( CompareOWString		( sFirstValue_1$, sCompareValue_1$ ) = cDifferent ) then
			ErrorMessage	( "getByName() ... Fehler [Element 1 fehlerhaft]" )
			goto Test_NormalUse_XNameContainer_Error
		elseif ( CompareOWString	( sFirstValue_2$, sCompareValue_2$ ) = cDifferent ) then
			ErrorMessage	( "getByName() ... Fehler [Element 2 fehlerhaft]" )
			goto Test_NormalUse_XNameContainer_Error
		elseif ( CompareOWString	( sFirstValue_3$, sCompareValue_3$ ) = cDifferent ) then
			ErrorMessage	( "getByName() ... Fehler [Element 3 fehlerhaft]" )
			goto Test_NormalUse_XNameContainer_Error
		elseif ( CompareOWString	( sFirstValue_4$, sCompareValue_4$ ) = cDifferent ) then
			ErrorMessage	( "getByName() ... Fehler [Element 4 fehlerhaft]" )
			goto Test_NormalUse_XNameContainer_Error
		elseif ( CompareOWString	( sFirstValue_5$, sCompareValue_5$ ) = cDifferent ) then
			ErrorMessage	( "getByName() ... Fehler [Element 5 fehlerhaft]" )
			goto Test_NormalUse_XNameContainer_Error
		end if

		InfoMessage	( "getByName() ... OK" )

		' Jetzt wird versucht einige der Elemente in ihrem Wert zu veraendern.
		sNewValue_1$	= "NewValue 1"
		sNewValue_2$	= "NewValue 2"
		sNewValue_3$	= "NewValue 3"
		sNewValue_4$	= "NewValue 4"
		sNewValue_5$	= "NewValue 5"

		aDocumentProperties.replaceByName ( sItemName_1$, sNewValue_1$ )
		aDocumentProperties.replaceByName ( sItemName_2$, sNewValue_2$ )
		aDocumentProperties.replaceByName ( sItemName_3$, sNewValue_3$ )
		aDocumentProperties.replaceByName ( sItemName_4$, sNewValue_4$ )
		aDocumentProperties.replaceByName ( sItemName_5$, sNewValue_5$ )

		' Zur Kontrolle die Werte ausgeben. (Nur wichtig, wenn geloggt wird!)
		ShowNameContainer ( aDocumentProperties )

		' Dann die Werte wieder auslesen.
		sCompareValue_1$ = aDocumentProperties.getByName ( sItemName_1$ )
		sCompareValue_2$ = aDocumentProperties.getByName ( sItemName_2$ )
		sCompareValue_3$ = aDocumentProperties.getByName ( sItemName_3$ )
		sCompareValue_4$ = aDocumentProperties.getByName ( sItemName_4$ )
		sCompareValue_5$ = aDocumentProperties.getByName ( sItemName_5$ )

		' Zum Vergleich die Werte ausgeben. (Nur wichtig, wenn geloggt wird!)
		ShowNameContainer ( aDocumentProperties )

		' Dann die Werte automatisch vergleichen!
		if ( CompareOWString		( sNewValue_1$, sCompareValue_1$ ) = cDifferent ) then
			ErrorMessage	( "replaceByName() ... Fehler [Element 1 fehlerhaft]" )
			goto Test_NormalUse_XNameContainer_Error
		elseif ( CompareOWString	( sNewValue_2$, sCompareValue_2$ ) = cDifferent ) then
			ErrorMessage	( "replaceByName() ... Fehler [Element 2 fehlerhaft]" )
			goto Test_NormalUse_XNameContainer_Error
		elseif ( CompareOWString	( sNewValue_3$, sCompareValue_3$ ) = cDifferent ) then
			ErrorMessage	( "replaceByName() ... Fehler [Element 3 fehlerhaft]" )
			goto Test_NormalUse_XNameContainer_Error
		elseif ( CompareOWString	( sNewValue_4$, sCompareValue_4$ ) = cDifferent ) then
			ErrorMessage	( "replaceByName() ... Fehler [Element 4 fehlerhaft]" )
			goto Test_NormalUse_XNameContainer_Error
		elseif ( CompareOWString	( sNewValue_5$, sCompareValue_5$ ) = cDifferent ) then
			ErrorMessage	( "replaceByName() ... Fehler [Element 5 fehlerhaft]" )
			goto Test_NormalUse_XNameContainer_Error
		end if

		InfoMessage	( "replaceByName() ... OK" )

		' Hier sollen einige der 5 Eintraege geloescht werden.
		aDocumentProperties.removeByName ( sItemName_1$ )
		aDocumentProperties.removeByName ( sItemName_3$ )

		' Dann wieder die Anzahl kontrollieren.
		' (Hier wird eine eigene BASIC-Hilfsfunktion verwendet! keine Interface-Methode)
		if ( getNameContainerCount ( aDocumentProperties ) <> 3 ) then
			ErrorMessage ( "removeByName() ... Fehler (Der NameConatiner enthaelt nicht die erwarteten 3 Elemente!)" )
			goto Test_NormalUse_XNameContainer_Error
		end if

		' Nun noch feststellen, ob die restlichen 3 denn auch tatsaechlich die richtigen sind.
		if ( aDocumentProperties.hasByName ( sItemName_2$ ) = FALSE ) then
			ErrorMessage	( "removeByName() ... Fehler [Element 2 nicht vorhanden!?]" )
			goto Test_NormalUse_XNameContainer_Error
		elseif ( aDocumentProperties.hasByName ( sItemName_4$ ) = FALSE ) then
			ErrorMessage	( "removeByName() ... Fehler [Element 4 nicht vorhanden!?]" )
			goto Test_NormalUse_XNameContainer_Error
		elseif ( aDocumentProperties.hasByName ( sItemName_5$ ) = FALSE ) then
			ErrorMessage	( "removeByName() ... Fehler [Element 5 nicht vorhanden!?]" )
			goto Test_NormalUse_XNameContainer_Error
		end if

		' Zur Kontrolle die Werte nochmals ausgeben. (Nur wichtig, wenn geloggt wird!)
		ShowNameContainer ( aDocumentProperties )

		InfoMessage	( "removeByName() ... OK" )

		' Der Test war erfolgreich! Meldung ausgeben und zurueck zm Aufrufer.
		' Ausschalten der Fehlerbehandlung
		on Error goto 0
		' Meldung ausgeben
		InfoMessage ( "Test_NormalUse_XNameContainer ... [ende]" )
		' Status setzen
		Test_NormalUse_XNameContainer = cOK
		' Und Funktion beenden
		Exit Function

' Es ist ein unerwartete Fehler aufgetreten! (Exception ...)
' Meldung ausgeben und mit Fehler zurueckkehren.
Test_NormalUse_XNameContainer_Error:
		' Ausschalten der Fehlerbehandlung
		on Error goto 0
		' Meldung ausgeben
		ErrorMessage ( "Test_NormalUse_XNameContainer ... [Error]" )
		' und Fehlerstatus setzen
		Test_NormalUse_XNameContainer = cError
		' Abbruch der Funktion erzwingen!
		Exit Function

End Function

'----------------------------------------------------------------------------
' Testmethode: Testen des unterstuetzten Interfaces "XPersist" unter normalen Bedingungen
'
' Returnwert = cOK		; wenn der Test erfolgreich war
' Returnwert = cError	; wenn sich das Objekt nicht korrekt verhalten hat
'----------------------------------------------------------------------------
Function Test_NormalUse_XPersist ( aDocumentProperties ) as Boolean

		' Einschalten der Fehlerbehandlung, um Exceptions oder aehnliches abzufangen!
		on Error goto Test_NormalUse_XPersist_Error

		InfoMessage ( "Test_NormalUse_XPersist ... [start]" )

		' Laden der Properties aus einer Datei
		aDocumentProperties.read ( cTestPath + "TestDebug_in.sdw" )

		' Zur Kontrolle anzeigen
		ShowProperties ( aDocumentProperties )

		' Speichern der Properties in einer neuen Datei
		aDocumentProperties.write	( cTestPath + "TestDebug_Out.sdw" )
		aDocumentProperties.read	( cTestPath + "TestDebug_Out.sdw" )

		' Zur Kontrolle anzeigen
		ShowProperties ( aDocumentProperties )
		
		' Der Test war erfolgreich! Meldung ausgeben und zurueck zm Aufrufer.
		' Ausschalten der Fehlerbehandlung
		on Error goto 0
		' Meldung ausgeben
		InfoMessage ( "Test_NormalUse_XPersist ... [ende]" )
		' Status setzen
		Test_NormalUse_XPersist = cOK
		' Und Funktion beenden
		Exit Function

' Es ist ein unerwartete Fehler aufgetreten! (Exception ...)
' Meldung ausgeben und mit Fehler zurueckkehren.
Test_NormalUse_XPersist_Error:
		' Ausschalten der Fehlerbehandlung
		on Error goto 0
		' Meldung ausgeben
		ErrorMessage ( "Test_NormalUse_XPersist ... [Error]" )
		' und Fehlerstatus setzen
		Test_NormalUse_XPersist = cError
		' Abbruch der Funktion erzwingen!
		Exit Function

End Function

'*****************************************************************************************************************
' Hilfsfunktionen und -methoden
'*****************************************************************************************************************

'----------------------------------------------------------------------------
' Hilfsmethode: Oeffnet die LOG-Datei.
'----------------------------------------------------------------------------
Sub OpenLOG ( sFileName$ )
		if ( bLOGOn = cOn ) then
			sLOGFileName$	= sFileName$
			nLOGFileHandle%	= FreeFile
			open sLOGFileName$ for output as nLOGFileHandle%
		end if
End Sub

'----------------------------------------------------------------------------
' Hilfsmethode: Schliesst die LOG-Datei.
'----------------------------------------------------------------------------
Sub CloseLOG
		if ( bLOGOn = cOn ) then
			close #nLOGFileHandle%
		end if
End Sub

'----------------------------------------------------------------------------
' Hilfsmethode: Gibt einen Text in einer LOG-Datei aus.
'----------------------------------------------------------------------------
Sub WriteLOG ( sMessage$ )
		if ( bLOGOn = cOn ) then
			Write #nLOGFileHandle% sMessage$
		end if
End Sub

'----------------------------------------------------------------------------
' Hilfsmethode: Gibt eine MessageBox mit Fehlertext, Zeilennummer und Warnschild aus.
'----------------------------------------------------------------------------
Sub ErrorMessage ( sMessage$ )
		' Entweder in die LOG-Datei schreiben oder eine MessageBox anzeigen.
		if ( bLOGOn = cOn ) then
			WriteLOG ( sMessage$ )
		else
			MsgBox ( sMessage$, 16 )
		end if
End Sub

'----------------------------------------------------------------------------
' Hilfsmethode: Gibt eine Hinweisbox aus.
'----------------------------------------------------------------------------
Sub InfoMessage ( sMessage$ )
		' Nur was anzeigen, wenn Nutzer es wuenscht!
		if ( bShowErrorsOnly = cOff ) then
			' Ansonsten wird entweder in die LOG-Datei geschrieben oder eine MessageBox angezeigt.
			if ( bLOGOn = cOn ) then
				WriteLOG ( sMessage$ )
			else
				MsgBox ( sMessage$, 64 )
			end if
		end if
End Sub

'----------------------------------------------------------------------------
' Hilfsfunktion: Vergleicht zwei OWString-Werte
'
' Returnwert = cEqual		; wenn Werte identisch sind
' Returnwert = cDifferent	; wenn Werte verschieden sind
'----------------------------------------------------------------------------
Function CompareOWString ( sOWString_1$, sOWString_2$ ) as Boolean

		if ( sOWString_1$ = sOWString_2$ ) then
			CompareOWString = cEqual
		else
			CompareOWString = cDifferent
		end if

End Function

'----------------------------------------------------------------------------
' Hilfsfunktion: Vergleicht zwei DateTime-Strukturen
'
' Returnwert = cEqual		; wenn Werte identisch sind
' Returnwert = cDifferent	; wenn Werte verschieden sind
'----------------------------------------------------------------------------
Function CompareDateTime ( aDateTime_1, aDateTime_2 ) as Boolean

		if ( aDateTime_1.Day = aDateTime_2.Day and aDateTime_1.Month = aDateTime_2.Month and aDateTime_1.Year = aDateTime_2.Year and aDateTime_1.Hours = aDateTime_1.Hours and aDateTime_1.Minutes = aDateTime_1.Minutes and aDateTime_1.Seconds = aDateTime_1.Seconds and aDateTime_1.HundredthSeconds = aDateTime_1.HundredthSeconds ) then
			CompareDateTime = cEqual
		else
			CompareDateTime = cDifferent
		end if

End Function

'----------------------------------------------------------------------------
' Hilfsfunktion: Vergleicht zwei ByteSequence's
'
' Returnwert = cEqual		; wenn Werte identisch sind
' Returnwert = cDifferent	; wenn Werte verschieden sind
'----------------------------------------------------------------------------
Function CompareByteSequence ( seqByteSequence_1, seqByteSequence_2 ) as Boolean

		' Wenn beide leer sind, sind sie auch identisch !
		' Dieser Test mit "IsArray" ist noetig, da bei einem leeren Array die
		' Funktion "UBound" einen Fehler produziert!
		if ( IsArray ( seqByteSequence_1 ) = FALSE and IsArray ( seqByteSequence_2 ) = FALSE ) then
			CompareByteSequence = cEqual
			Exit Function
		end if

		' Wenn jedoch nur eine leer ist, dann sind sie nicht identisch.
		if ( IsArray ( seqByteSequence_1 ) = FALSE ) or ( IsArray ( seqByteSequence_2 ) = FALSE ) then
			CompareByteSequence = cDifferent
			Exit Function
		end if

		' Besorgen der Anzahl der Elemente der Sequences
		nElementCount_1%	=	UBound ( seqByteSequence_1 )
		nElementCount_2%	=	UBound ( seqByteSequence_2 )

		' Wenn diese Anzahl schon verschieden ist, dann ...
		if ( nElementCount_1% <> nElementCount_2% ) then
			' ... sind die Sequences wohl verschieden.
			CompareByteSequence = cDifferent
			' Die Element brauchen dann nicht mehr verglichen zu werden.
			Exit Function
		end if

		' Ansonsten werden die Elemente einzeln miteinander verglichen.
		for nCounter%=0 to nElementCount_1% step 1
			' Wenn auch nur ein paar davon verschieden ist, dann ...
			if ( nElementCount_1%(nCounter%) <> nElementCount_2%(nCounter%) ) then
				' ... kann der Vergleich abgebrochen werden!
				CompareByteSequence = cDifferent
				Exit Function
			end if
		next nCounter%

		' Wenn man bis hier gekommen ist, dann sind die Sequences identisch.
		CompareByteSequence = cEqual

End Function

'----------------------------------------------------------------------------
' Hilfsfunktion: Vergleicht zwei Int16-Werte
'
' Returnwert = cEqual		; wenn Werte identisch sind
' Returnwert = cDifferent	; wenn Werte verschieden sind
'----------------------------------------------------------------------------
Function CompareInt16 ( nInt16_1%, nInt16_2% ) as Boolean

		if ( nInt16_1% = nInt16_2% ) then
			CompareInt16 = cEqual
		else
			CompareInt16 = cDifferent
		end if

End Function

'----------------------------------------------------------------------------
' Hilfsfunktion: Vergleicht zwei Int32-Werte
'
' Returnwert = cEqual		; wenn Werte identisch sind
' Returnwert = cDifferent	; wenn Werte verschieden sind
'----------------------------------------------------------------------------
Function CompareInt32 ( nInt32_1&, nInt32_2& ) as Boolean

		if ( nInt32_1& = nInt32_2& ) then
			CompareInt32 = cEqual
		else
			CompareInt32 = cDifferent
		end if

End Function

'----------------------------------------------------------------------------
' Hilfsfunktion: Vergleicht zwei Bool-Werte
'
' Returnwert = cEqual		; wenn Werte identisch sind
' Returnwert = cDifferent	; wenn Werte verschieden sind
'----------------------------------------------------------------------------
Function CompareBool ( bBool_1, bBool_2 ) as Boolean

		if ( bBool_1 = bBool_2 ) then
			CompareBool = cEqual
		else
			CompareBool = cDifferent
		end if

End Function

'----------------------------------------------------------------------------
' Hilfsfunktion: Vergleicht die Properties zweier Objekte um Unterschiede festzustellen.
'
' Returnwert = cEqual		; wenn Objekte von den Properties her identisch sind
' Returnwert = cDifferent	; wenn Objekte von den Properties her verschieden sind
'----------------------------------------------------------------------------
Function CompareDocumentProperties ( aDocumentProperties_1, aDocumentProperties_2 ) as Boolean

		' Besorgen der Werte und zwischenspeichern (bezogen auf Objekt 1)
		sAuthor_1$					= aDocumentProperties_1.getPropertyValue ( "Author"						)
		bAutoloadEnabled_1			= aDocumentProperties_1.getPropertyValue ( "AutoloadEnabled"			)
		nAutoloadSecs_1%			= aDocumentProperties_1.getPropertyValue ( "AutoloadSecs"				)
		sAutoLoadURL_1$				= aDocumentProperties_1.getPropertyValue ( "AutoloadURL"				)
		sBliendCopiesTo_1$			= aDocumentProperties_1.getPropertyValue ( "BlindCopiesTo"				)
		sCopiesTo_1$				= aDocumentProperties_1.getPropertyValue ( "CopiesTo"					)
		aCreationDate_1				= aDocumentProperties_1.getPropertyValue ( "CreationDate"				)
		sDefaultTarget_1$			= aDocumentProperties_1.getPropertyValue ( "DefaultTarget"				)
		sDescription_1$				= aDocumentProperties_1.getPropertyValue ( "Description"				)
		nEditingCycles_1%			= aDocumentProperties_1.getPropertyValue ( "EditingCycles"				)
		nEditingDuration_1&			= aDocumentProperties_1.getPropertyValue ( "EditingDuration"			)
		seqExtraData_1				= aDocumentProperties_1.getPropertyValue ( "ExtraData"					)
		sInReplyTo_1$				= aDocumentProperties_1.getPropertyValue ( "InReplyTo"					)
		bIsEncrypted_1				= aDocumentProperties_1.getPropertyValue ( "IsEncrypted"				)
		sKeywords_1$				= aDocumentProperties_1.getPropertyValue ( "Keywords"					)
		sMIMEType_1$				= aDocumentProperties_1.getPropertyValue ( "MIMEType"					)
		sModifiedBy_1$				= aDocumentProperties_1.getPropertyValue ( "ModifiedBy"					)
		aModifyDate_1				= aDocumentProperties_1.getPropertyValue ( "ModifyDate"					)
		sNewsgroups_1$				= aDocumentProperties_1.getPropertyValue ( "Newsgroups"					)
		sOriginal_1$				= aDocumentProperties_1.getPropertyValue ( "Original"					)
		bPortableGraphics_1			= aDocumentProperties_1.getPropertyValue ( "PortableGraphics"			)
		aPrintDate_1				= aDocumentProperties_1.getPropertyValue ( "PrintDate"					)
		sPrintedBy_1$				= aDocumentProperties_1.getPropertyValue ( "PrintedBy"					)
		nPriority_1%				= aDocumentProperties_1.getPropertyValue ( "Priority"					)
		bQueryTemplate_1			= aDocumentProperties_1.getPropertyValue ( "QueryTemplate"				)
		sRecipient_1$				= aDocumentProperties_1.getPropertyValue ( "Recipient"					)
		sReferences_1$				= aDocumentProperties_1.getPropertyValue ( "References"					)
		sReplyTo_1$					= aDocumentProperties_1.getPropertyValue ( "ReplyTo"					)
		bSaveGraphicsCompressed_1	= aDocumentProperties_1.getPropertyValue ( "SaveGraphicsCompressed"		)
		bSaveOriginalGraphics_1		= aDocumentProperties_1.getPropertyValue ( "SaveOriginalGraphics"		)
		bSaveVersionOnClose_1		= aDocumentProperties_1.getPropertyValue ( "SaveVersionOnClose"			)
		sTemplate_1$				= aDocumentProperties_1.getPropertyValue ( "Template"					)
		bTemplateConfig_1			= aDocumentProperties_1.getPropertyValue ( "TemplateConfig"				)
		aTemplateDate_1				= aDocumentProperties_1.getPropertyValue ( "TemplateDate"				)
		sTemplateFileName_1$		= aDocumentProperties_1.getPropertyValue ( "TemplateFileName"			)
		sTheme_1$					= aDocumentProperties_1.getPropertyValue ( "Theme"						)
		sTitle_1$					= aDocumentProperties_1.getPropertyValue ( "Title"						)
		bUserData_1					= aDocumentProperties_1.getPropertyValue ( "UserData"					)

		' Besorgen der Werte und zwischenspeichern (bezogen auf Objekt 2)
		sAuthor_2$					= aDocumentProperties_2.getPropertyValue ( "Author"						)
		bAutoloadEnabled_2			= aDocumentProperties_2.getPropertyValue ( "AutoloadEnabled"			)
		nAutoloadSecs_2%			= aDocumentProperties_2.getPropertyValue ( "AutoloadSecs"				)
		sAutoLoadURL_2$				= aDocumentProperties_2.getPropertyValue ( "AutoloadURL"				)
		sBliendCopiesTo_2$			= aDocumentProperties_2.getPropertyValue ( "BlindCopiesTo"				)
		sCopiesTo_2$				= aDocumentProperties_2.getPropertyValue ( "CopiesTo"					)
		aCreationDate_2				= aDocumentProperties_2.getPropertyValue ( "CreationDate"				)
		sDefaultTarget_2$			= aDocumentProperties_2.getPropertyValue ( "DefaultTarget"				)
		sDescription_2$				= aDocumentProperties_2.getPropertyValue ( "Description"				)
		nEditingCycles_2%			= aDocumentProperties_2.getPropertyValue ( "EditingCycles"				)
		nEditingDuration_2&			= aDocumentProperties_2.getPropertyValue ( "EditingDuration"			)
		seqExtraData_2				= aDocumentProperties_2.getPropertyValue ( "ExtraData"					)
		sInReplyTo_2$				= aDocumentProperties_2.getPropertyValue ( "InReplyTo"					)
		bIsEncrypted_2				= aDocumentProperties_2.getPropertyValue ( "IsEncrypted"				)
		sKeywords_2$				= aDocumentProperties_2.getPropertyValue ( "Keywords"					)
		sMIMEType_2$				= aDocumentProperties_2.getPropertyValue ( "MIMEType"					)
		sModifiedBy_2$				= aDocumentProperties_2.getPropertyValue ( "ModifiedBy"					)
		aModifyDate_2				= aDocumentProperties_2.getPropertyValue ( "ModifyDate"					)
		sNewsgroups_2$				= aDocumentProperties_2.getPropertyValue ( "Newsgroups"					)
		sOriginal_2$				= aDocumentProperties_2.getPropertyValue ( "Original"					)
		bPortableGraphics_2			= aDocumentProperties_2.getPropertyValue ( "PortableGraphics"			)
		aPrintDate_2				= aDocumentProperties_2.getPropertyValue ( "PrintDate"					)
		sPrintedBy_2$				= aDocumentProperties_2.getPropertyValue ( "PrintedBy"					)
		nPriority_2%				= aDocumentProperties_2.getPropertyValue ( "Priority"					)
		bQueryTemplate_2			= aDocumentProperties_2.getPropertyValue ( "QueryTemplate"				)
		sRecipient_2$				= aDocumentProperties_2.getPropertyValue ( "Recipient"					)
		sReferences_2$				= aDocumentProperties_2.getPropertyValue ( "References"					)
		sReplyTo_2$					= aDocumentProperties_2.getPropertyValue ( "ReplyTo"					)
		bSaveGraphicsCompressed_2	= aDocumentProperties_2.getPropertyValue ( "SaveGraphicsCompressed"		)
		bSaveOriginalGraphics_2		= aDocumentProperties_2.getPropertyValue ( "SaveOriginalGraphics"		)
		bSaveVersionOnClose_2		= aDocumentProperties_2.getPropertyValue ( "SaveVersionOnClose"			)
		sTemplate_2$				= aDocumentProperties_2.getPropertyValue ( "Template"					)
		bTemplateConfig_2			= aDocumentProperties_2.getPropertyValue ( "TemplateConfig"				)
		aTemplateDate_2				= aDocumentProperties_2.getPropertyValue ( "TemplateDate"				)
		sTemplateFileName_2$		= aDocumentProperties_2.getPropertyValue ( "TemplateFileName"			)
		sTheme_2$					= aDocumentProperties_2.getPropertyValue ( "Theme"						)
		sTitle_2$					= aDocumentProperties_2.getPropertyValue ( "Title"						)
		bUserData_2					= aDocumentProperties_2.getPropertyValue ( "UserData"					)

		' Als erwarteten Zielwert schon mal "Properties identisch" annehmen!!!
		' Für den Fall, das nur eine diesen Anspruch nicht erfüllt, wird der Wert einfach zurückgesetzt.
		' Von da bleibt der neue Wert bestehen und zeigt an, daß sich mindestens eine Property geändert hat!
		CompareDocumentProperties = cEqual

		' Dann die Werte vergleichen
		if ( CompareOWString		( sAuthor_1$,					sAuthor_2$					) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareBool		( bAutoloadEnabled_1,			bAutoloadEnabled_2			) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareInt16		( nAutoloadSecs_1%,				nAutoloadSecs_2%			) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sAutoLoadURL_1$,				sAutoLoadURL_2$				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sBliendCopiesTo_1$,			sBliendCopiesTo_2$			) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sCopiesTo_1$,					sCopiesTo_2$				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareDateTime	( aCreationDate_1,				aCreationDate_2				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sDefaultTarget_1$,			sDefaultTarget_2$			) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sDescription_1$,				sDescription_2$				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareInt16		( nEditingCycles_1%,			nEditingCycles_2%			) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareInt32		( nEditingDuration_1&,			nEditingDuration_2&			) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareByteSequence( seqExtraData_1,				seqExtraData_2				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sInReplyTo_1$,				sInReplyTo_2$				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareBool		( bIsEncrypted_1,				bIsEncrypted_2				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sKeywords_1$,					sKeywords_2$				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sMIMEType_1$,					sMIMEType_2$				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sModifiedBy_1$,				sModifiedBy_2$				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareDateTime	( aModifyDate_1,				aModifyDate_2				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sNewsgroups_1$,				sNewsgroups_2$				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sOriginal_1$,					sOriginal_2$				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareBool		( bPortableGraphics_1,			bPortableGraphics_2			) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareDateTime	( aPrintDate_1,					aPrintDate_2				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sPrintedBy_1$,				sPrintedBy_2$				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareInt16		( nPriority_1%,					nPriority_2%				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareBool		( bQueryTemplate_1,				bQueryTemplate_2			) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sRecipient_1$,				sRecipient_2$				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sReferences_1$,				sReferences_2$				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sReplyTo_1$,					sReplyTo_2$					) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareBool		( bSaveGraphicsCompressed_1,	bSaveGraphicsCompressed_2	) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareBool		( bSaveOriginalGraphics_1,		bSaveOriginalGraphics_2		) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareBool		( bSaveVersionOnClose_1,		bSaveVersionOnClose_2		) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sTemplate_1$,					sTemplate_2$				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareBool		( bTemplateConfig_1,			bTemplateConfig_2			) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareDateTime	( aTemplateDate_1,				aTemplateDate_2				) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sTemplateFileName_1$,			sTemplateFileName_2$		) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sTheme_1$,					sTheme_2$					) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareOWString	( sTitle_1$,					sTitle_2$					) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		elseif ( CompareBool		( bUserData_1,					bUserData_2					) = cDifferent ) then
			CompareDocumentProperties = cDifferent
		end if

End Function

'----------------------------------------------------------------------------
' Hilfsfunktion: Wandelt eine Struktur des Types DateTime in einen formatierten String um
'----------------------------------------------------------------------------
Function DateTime2String ( aDateTime ) as String

		stempString$ = ""
		stempString$ = stempString$ + aDateTime.Day					+ "."
		stempString$ = stempString$ + aDateTime.Month				+ "."
		stempString$ = stempString$ + aDateTime.Year				+ " - "
		stempString$ = stempString$ + aDateTime.Hours				+ ":"
		stempString$ = stempString$ + aDateTime.Minutes				+ ":"
		stempString$ = stempString$ + aDateTime.Seconds				+ ":"
		stempString$ = stempString$ + aDateTime.HundredthSeconds	+ " Uhr"

		DateTime2String = stempString$

End Function

'----------------------------------------------------------------------------
' Hilfsfunktion: Wandelt eine Sequence von Bytes in einen formatierten String um
'----------------------------------------------------------------------------
Function ByteSequence2String ( seqByteSequence ) as String

		nElementCount% = UBound ( seqByteSequence() )

		if ( nElementCount% < 1 ) then
			stempString$ = "leer"
		else
			stempString$ = "{"
			for nCounter%=0 to nElementCount% step 1
				stempString$ = stempString$ + seqByteSequence(nCounter%)
			next nCounter%
			stempString$ = stempString$ + "}"
		end if

		ByteSequence2String = stempString$

End Function

'----------------------------------------------------------------------------
' Hilfsmethode: Zeigt die aktuellen Werte ALLER Properties an
'----------------------------------------------------------------------------
Sub ShowProperties ( aDocumentProperties )

		' Besorgen der Werte und zwischenspeichern
		sAuthor$				= aDocumentProperties.getPropertyValue ( "Author"					)
		bAutoloadEnabled		= aDocumentProperties.getPropertyValue ( "AutoloadEnabled"			)
		nAutoloadSecs%			= aDocumentProperties.getPropertyValue ( "AutoloadSecs"				)
		sAutoLoadURL$			= aDocumentProperties.getPropertyValue ( "AutoloadURL"				)
		sBliendCopiesTo$		= aDocumentProperties.getPropertyValue ( "BlindCopiesTo"			)
		sCopiesTo$				= aDocumentProperties.getPropertyValue ( "CopiesTo"					)
		dCreationDate			= aDocumentProperties.getPropertyValue ( "CreationDate"				)
		sDefaultTarget$			= aDocumentProperties.getPropertyValue ( "DefaultTarget"			)
		sDescription$			= aDocumentProperties.getPropertyValue ( "Description"				)
		nEditingCycles%			= aDocumentProperties.getPropertyValue ( "EditingCycles"			)
		nEditingDuration&		= aDocumentProperties.getPropertyValue ( "EditingDuration"			)
		seqExtraData			= aDocumentProperties.getPropertyValue ( "ExtraData"				)
		sInReplyTo$				= aDocumentProperties.getPropertyValue ( "InReplyTo"				)
		bIsEncrypted			= aDocumentProperties.getPropertyValue ( "IsEncrypted"				)
		sKeywords$				= aDocumentProperties.getPropertyValue ( "Keywords"					)
		sMIMEType$				= aDocumentProperties.getPropertyValue ( "MIMEType"					)
		sModifiedBy$			= aDocumentProperties.getPropertyValue ( "ModifiedBy"				)
		dModifyDate				= aDocumentProperties.getPropertyValue ( "ModifyDate"				)
		sNewsgroups$			= aDocumentProperties.getPropertyValue ( "Newsgroups"				)
		sOriginal$				= aDocumentProperties.getPropertyValue ( "Original"					)
		bPortableGraphics		= aDocumentProperties.getPropertyValue ( "PortableGraphics"			)
		dPrintDate				= aDocumentProperties.getPropertyValue ( "PrintDate"				)
		sPrintedBy$				= aDocumentProperties.getPropertyValue ( "PrintedBy"				)
		nPriority%				= aDocumentProperties.getPropertyValue ( "Priority"					)
		bQueryTemplate			= aDocumentProperties.getPropertyValue ( "QueryTemplate"			)
		sRecipient$				= aDocumentProperties.getPropertyValue ( "Recipient"				)
		sReferences$			= aDocumentProperties.getPropertyValue ( "References"				)
		sReplyTo$				= aDocumentProperties.getPropertyValue ( "ReplyTo"					)
		bSaveGraphicsCompressed	= aDocumentProperties.getPropertyValue ( "SaveGraphicsCompressed"	)
		bSaveOriginalGraphics	= aDocumentProperties.getPropertyValue ( "SaveOriginalGraphics"		)
		bSaveVersionOnClose		= aDocumentProperties.getPropertyValue ( "SaveVersionOnClose"		)
		sTemplate$				= aDocumentProperties.getPropertyValue ( "Template"					)
		bTemplateConfig			= aDocumentProperties.getPropertyValue ( "TemplateConfig"			)
		dTemplateDate			= aDocumentProperties.getPropertyValue ( "TemplateDate"				)
		sTemplateFileName$		= aDocumentProperties.getPropertyValue ( "TemplateFileName"			)
		sTheme$					= aDocumentProperties.getPropertyValue ( "Theme"					)
		sTitle$					= aDocumentProperties.getPropertyValue ( "Title"					)
		bUserData				= aDocumentProperties.getPropertyValue ( "UserData"					)

		' Eine Zeichenkette zusammenbasteln, welche die Werte formatiert darstellt.
		sOutLine$ = 			"[OWString]"		+ chr$(9) + "Author"  					+ chr$(9) + "= {" + chr$(9) + sAuthor$							+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[sal_Bool]"		+ chr$(9) + "AutoloadEnabled"			+ chr$(9) + "= {" + chr$(9) + bAutoloadEnabled					+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[sal_Int16]"		+ chr$(9) + "AutoloadSecs"				+ chr$(9) + "= {" + chr$(9) + nAutoloadSecs%					+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "AutoLoadURL"				+ chr$(9) + "= {" + chr$(9) + sAutoLoadURL$						+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "BliendCopiesTo"			+ chr$(9) + "= {" + chr$(9) + sBliendCopiesTo$					+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "CopiesTo"					+ chr$(9) + "= {" + chr$(9) + sCopiesTo$						+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[DateTime]"		+ chr$(9) + "CreationDate"				+ chr$(9) + "= {" + chr$(9) + DateTime2String(dCreationDate)	+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "DefaultTarget"				+ chr$(9) + "= {" + chr$(9) + sDefaultTarget$					+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "Description"				+ chr$(9) + "= {" + chr$(9) + sDescription$						+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[sal_Int16]"		+ chr$(9) + "EditingCycles"				+ chr$(9) + "= {" + chr$(9) + nEditingCycles%					+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[sal_Int32]"		+ chr$(9) + "EditingDuration"			+ chr$(9) + "= {" + chr$(9) + nEditingDuration&					+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[Sequence<Byte>]"	+ chr$(9) + "ExtraData"					+ chr$(9) + "= {" + chr$(9) + ByteSequence2String(seqExtraData)	+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "InReplyTo"					+ chr$(9) + "= {" + chr$(9) + sInReplyTo$						+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[sal_Bool]"		+ chr$(9) + "IsEncrypted"				+ chr$(9) + "= {" + chr$(9) + bIsEncrypted						+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "Keywords"					+ chr$(9) + "= {" + chr$(9) + sKeywords$						+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "MIMEType"					+ chr$(9) + "= {" + chr$(9) + sMIMEType$						+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "ModifiedBy"				+ chr$(9) + "= {" + chr$(9) + sModifiedBy$						+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[DateTime]"		+ chr$(9) + "ModifyDate"				+ chr$(9) + "= {" + chr$(9) + DateTime2String(dModifyDate)		+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "Newsgroups"				+ chr$(9) + "= {" + chr$(9) + sNewsgroups$						+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "Original"					+ chr$(9) + "= {" + chr$(9) + sOriginal$						+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[sal_Bool]"		+ chr$(9) + "PortableGraphics"			+ chr$(9) + "= {" + chr$(9) + bPortableGraphics					+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[DateTime]"		+ chr$(9) + "PrintDate"					+ chr$(9) + "= {" + chr$(9) + DateTime2String(dPrintDate)		+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "PrintedBy"					+ chr$(9) + "= {" + chr$(9) + sPrintedBy$						+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[sal_Int16]"		+ chr$(9) + "Priority"					+ chr$(9) + "= {" + chr$(9) + nPriority%						+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[sal_Bool]"		+ chr$(9) + "QueryTemplate"				+ chr$(9) + "= {" + chr$(9) + bQueryTemplate					+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "Recipient"					+ chr$(9) + "= {" + chr$(9) + sRecipient$						+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "References"				+ chr$(9) + "= {" + chr$(9) + sReferences$						+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "ReplyTo"					+ chr$(9) + "= {" + chr$(9) + sReplyTo$							+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[sal_Bool]"		+ chr$(9) + "SaveGraphicsCompressed"	+ chr$(9) + "= {" + chr$(9) + bSaveGraphicsCompressed			+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[sal_Bool]"		+ chr$(9) + "SaveOriginalGraphics"		+ chr$(9) + "= {" + chr$(9) + bSaveOriginalGraphics				+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[sal_Bool]"		+ chr$(9) + "SaveVersionOnClose"		+ chr$(9) + "= {" + chr$(9) + bSaveVersionOnClose				+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "Template"					+ chr$(9) + "= {" + chr$(9) + sTemplate$						+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[sal_Bool]"		+ chr$(9) + "TemplateConfig"			+ chr$(9) + "= {" + chr$(9) + bTemplateConfig					+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[DateTime]"		+ chr$(9) + "TemplateDate"				+ chr$(9) + "= {" + chr$(9) + DateTime2String(dTemplateDate)	+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "TemplateFileName"			+ chr$(9) + "= {" + chr$(9) + sTemplateFileName$				+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "Theme"						+ chr$(9) + "= {" + chr$(9) + sTheme$							+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[OWString]"		+ chr$(9) + "Title"						+ chr$(9) + "= {" + chr$(9) + sTitle$							+ "}" + chr$(13)
		sOutLine$ = sOutLine$ + "[sal_Bool]"		+ chr$(9) + "UserData"					+ chr$(9) + "= {" + chr$(9) + bUserData							+ "}"

		' Ausgabe der formatierten Zeichenkette
		InfoMessage ( sOutLine$ )
End Sub

'----------------------------------------------------------------------------
' Hilfsmethode: Zeigt die aktuellen Elemente des NameContainers an.
'----------------------------------------------------------------------------
Sub ShowNameContainer ( aDocumentProperties )

		if ( aDocumentProperties.hasElements () = FALSE ) then
			InfoMessage ( "Keine Elemente im NameContainer enthalten." )
			Exit Sub
		end if

		aNameField		= aDocumentProperties.getElementNames ()
		if ( IsArray ( aNameField ) = FALSE ) then
			ErrorMessage ( "getElementNames() .... Fehler (Es konnte keine Sequence bestimmt werden!)" )
			Exit Sub
		end if
		nElementCount%	= UBound ( aNameField () )

		stempString$ = ""
		for nCounter%=0 to nElementCount% step 1
			stempString$ = "[" + nCounter% + "]"
			stempString$ = stempString$ + chr$(9) + aNameField(nCounter%)
			stempString$ = stempString$ + chr$(9) + "="
			stempString$ = stempString$ + chr$(9) + aDocumentProperties.getByName ( aNameField(nCounter%) )
			stempString$ = stempString$ + chr$(13)
		next nCounter%

		InfoMessage ( stempString$ )

End Sub

'----------------------------------------------------------------------------
' Hilfsfunktion: Ermittelt die Anzahl der im NameContainer enthaltenen Elemente.
'
' Returnwert = Anzahl der Elemente
'----------------------------------------------------------------------------
Function getNameContainerCount ( aDocumentProperties ) as Long

		if ( aDocumentProperties.hasElements () = FALSE ) then
			getNameContainerCount = 0
			Exit Function
		end if

		aNameField		= aDocumentProperties.getElementNames ()
		nElementCount%	= UBound ( aNameField () )
		
		' Da die Zaehlung bei 0 beginnt, und der ermittelte Wert die obere Grenze darstellt,
		' muss hier eine 1 draufgeschlagen werden.
		getNameContainerCount = nElementCount% + 1

End Function