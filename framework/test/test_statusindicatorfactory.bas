Sub Main

	rem Get reference to current active frame. Most time this will be
	rem the basic ide by himself.
	xTestFrame = StarDesktop.ActiveFrame

	rem Create more then one indicator objects for this frame.
	xIndicator1 = xTestFrame.createStatusIndicator()
	xIndicator2 = xTestFrame.createStatusIndicator()
	xIndicator3 = xTestFrame.createStatusIndicator()

	rem Check status of creation. No null references should be detected.
	if( isNull(xIndicator1)=TRUE ) or ( isNull(xIndicator2)=TRUE ) or ( isNull(xIndicator3)=TRUE ) then
		msgbox "Error: Could not create status indicators!"
		exit Sub
	endif

	rem Start working for indicator 1 and 2.
	rem The window should NOT be shown!
	xIndicator1.start( "Indicator 1:", 100 )
	xIndicator2.start( "Indicator 2:", 200 )
	msgbox "Indicator 1 and 2 was started ... the window should NOT be shown!"

	rem Start working for indicator 3.
	rem The window should be shown! It's the most active one.
	xIndicator3.start( "Indicator 3:", 300 )
	msgbox "Indicator 3 was started ... the window should be shown!"

	rem Set different values and texts for indicator 1 and 2.
	rem These values are not visible.
	xIndicator1.setValue( 25 )
	xIndicator2.setValue( 50 )

	rem Work with indicator 3.
	rem If working finished automaticly indicator 2 is reactivated.
	i = 0
	while i<300
		xIndicator3.setText( "Indicator 3: Range=300 Value=" + i )
		xIndicator3.setValue( i )
		i = i+10
		wait( 1 )
	wend

	rem Delete indicator 2 before you deactivate number 3!
	rem The next automaticly activated indicator will be the number 1.
	xIndicator2.end
	msgbox "Indicator 3 will be destroyed. Indicator 2 was deleted ... number 1 must reactivated automaticly!"
	xIndicator3.end

	rem Work with indicator 1.
	rem If working finished automaticly the window will be destroyed.
	i = 25
	while i<100
		xIndicator1.setText( "Indicator 1: Range=100 Value=" + i )
		xIndicator1.setValue( i )
		i = i+10
		wait( 1 )
	wend
	xIndicator1.setText( "Indicator 1: ... reset values to defaults" )
	wait( 1000 )
	xIndicator1.reset
	xIndicator1.setText( "Indicator 1: ... set 50 % for progress" )
	wait( 1000 )
	xIndicator1.setValue( 50 )
	msgbox "Indicator 1 will be destroyed. Indicator window must destroyed automaticly!"
	xIndicator1.end

	msgbox "Test for status indicator finished successful!"
End Sub
