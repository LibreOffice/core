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
'/***********************************************************************
'*
'* owner : oliver.craemer@oracle.com
'*
'* short description : Functionality Test - Solver
'*
'************************************************************************
'*
' #1 tExampleCalculation
' #1 tExampleCalculation2
'*
'\***********************************************************************

testcase tExampleCalculation

	dim sdecimalseperator as string
	
	sDecimalseperator = GetDecimalSeperator

    printlog " Open testdocument 'gTesttoolPath + spreadsheet\optional\input\Solver_testdoc01.ods'"
    hFileOpenLocally( gTesttoolPath & "spreadsheet\optional\input\Solver_testdoc01.ods" )
	printlog " Tools - Solver"
	ToolsSolver
	Kontext "Solver"
	printlog " Set target cell to $E$15"
	TargetCell.SetText "$E$15"
	printlog " Set Optimize result to Minimum"
	OptimizeMinimum.Check
	printlog " Set 'By changing cells' to $G$13:$G$14"
	ChangingCells.SetText "$G$13:$G$14"
	printlog " Set the first cell reference under 'Limiting conditions' to $C$15, the first Operator to '<=' and the first Value to 4"
	CellReference1.SetText "$C$15"
	Operator1.Select 1
	Value1.SetText "4"
	printlog " Set the second cell reference under 'Limiting conditions' to $D$15, the second Operator to '>=' and the second Value to 10"
	CellReference2.SetText "$D$15"
	Operator2.Select 3
	Value2.SetText "10"
	printlog " Set the third cell reference under 'Limiting conditions' to $G$15, the third Operator to '=' and the third Value to 300."
	CellReference3.SetText "$G$15"
	Operator3.Select 2
	Value3.SetText "300"
	printlog " Click on Solve"
	Solve.Click
	printlog " A new dialog opens informing you that a result was found."
	Kontext "SolvingResult"
	if not SolvingResult.exists then
        warnlog "Normally there should be a Success dialog, but there is none"
		Kontext "NoSolution"
		if NoSolution.exists then
			NoSolution.OK
			Kontext "Solver"
			Solver.Cancel
			Call hCloseDocument
			goto endsub
		else 
			Kontext "Solver"
			Solver.Cancel
			Call hCloseDocument
			goto endsub
		end if
    end if
	printlog " Click on the 'Restore Previous' Button. The Solver dialog is shown again."
	SolvingResult.Cancel
	Kontext "Solver"
	printlog " Press ALT+S . The Solved dialog opens again. Press the Enter key."
	Solve.Click
	Kontext "SolvingResult"
	SolvingResult.OK
	Kontext "DocumentCalc"
	printlog " C15 must show 4.0000 mg, D15 must show 18.0000 mg, E15 must show 22.0000 ct, G13 is set to 120, G14 shows 180 and therefore G15 300."
	call fCalcCompareCellValue ("C15","4" & sDecimalseperator & "0000 mg")
	call fCalcCompareCellValue ("D15","18" & sDecimalseperator & "0000 mg")
	call fCalcCompareCellValue ("E15","22" & sDecimalseperator & "0000 ct")
	call fCalcCompareCellValue ("G13","120")
	call fCalcCompareCellValue ("G14","180")
	call fCalcCompareCellValue ("G15","300")
	printlog " Tools - Solver"
	ToolsSolver
	Kontext "Solver"
	printlog " Change limitatation value 3 from 300 to 500"
	Value3.SetText "500"
	printlog " Press ALT+S to solve the problem and keep the results."
	Solve.Click
	Kontext "SolvingResult"
	SolvingResult.OK
	Kontext "DocumentCalc"
	printlog " The results should be :C15 must show 4.0000 mg, D15 must show 51.3333 mg, E15 must show 68.6667 ct, G13 is set to 520, G14 shows -20 and therefore G15 500"
	call fCalcCompareCellValue ("C15","4" & sDecimalseperator & "0000 mg")
	call fCalcCompareCellValue ("D15","51" & sDecimalseperator & "3333 mg")
	call fCalcCompareCellValue ("E15","68" & sDecimalseperator & "6667 ct")
	call fCalcCompareCellValue ("G13","520")
	call fCalcCompareCellValue ("G14","-20")
	call fCalcCompareCellValue ("G15","500")
	printlog " So the solution found is correct but not usefull. Let's limit the lemonade to positive values by adding a new limitation."
	printlog " Tools - Solver"
	ToolsSolver
	kontext "Solver"
	printlog " Set the fourth cell reference under 'Limiting conditions' to $G$14, the fourth Operator to '>=' and the fourth Value to 0."
	CellReference4.SetText "$G$14"
	Operator4.Select 3
	Value4.SetText "0"
	printlog " Click on Solve."
	Solve.Click
	printlog " The No Solution dialog opens, telling you that the model is not solvable with the given limiting conditions."
	Kontext "NoSolution"
	if not NoSolution.exists then
		warnlog "Normally the NoSolution dialog appears, but there is none"
	end if
	printlog " Click on Ok at this dialog, the Solver dialog should be shown."
	NoSolution.OK
	Kontext "Solver"
	Solver.Close
	printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tExampleCalculation2
	
    printlog " Open testdocument 'gTesttoolPath + spreadsheet\optional\input\stest_large.ods'"
    hFileOpenLocally( gTesttoolPath & "spreadsheet\optional\input\stest_large.ods" )
	printlog " Tools - Solver"
	ToolsSolver
	Kontext "Solver"
	printlog " Target Cell: $Y$2"
	TargetCell.SetText "$Y$2"
	printlog " Optimize to: Maximum"
	OptimizeMaximum.Check
	printlog " Changing cells: $B$5:$B$134"
	ChangingCells.SetText "$B$5:$B$134"
	printlog " Limiting Conditions: Cell reference: $D$2:$W$2 Operator : <= Value: $D$3:$W$3"
	CellReference1.SetText "$D$2:$W$2"
	Operator1.Select 1
	Value1.SetText "$D$3:$W$3"
	printlog " Copy the following String to the second cell reference input line: $D$2:$W$2 <= $D$3:$W$3"
	CellReference2.SetText "$D$2:$W$2"
	Operator1.Select 2
	Value2.SetText "$D$3:$W$3"
	printlog " Click on Solve"
	Solve.Click
	printlog " You get a new Warning dialog talking about Invalid condition."
	Kontext "NoSolution"
	if not NoSolution.exists then
		warnlog "Normally there should be a No Solution dialog"
	end if
	NoSolution.OK
	Kontext "Solver"
	printlog " Click on the remove Button next to the second Value field to remove the faulty condition."
	Remove2.Click
	if Value2 = "$D$3:$W$3" then
		warnlog "The condition was not removed"
	end if
	printlog " Click on Options button, the Options dialog opens"
	Options.Click
	Kontext "SolverOptions"
	if not SolverOptions.exists then
		warnlog "The Options dialog is not open"
	end if
	printlog " Tag 'Assume variables as integer'"
	SolverSettings.Select (1)
	SolverSettings.TypeKeys "<SPACE>"
	printlog " Tag 'Assume variables not negative'"
	SolverSettings.Select (2)
	SolverSettings.TypeKeys "<SPACE>"
	printlog " Select SolvingTime"
	SolverSettings.Select (5)
	printlog " Click on Edit button to open the Editdialog"
	SolverSettingsEdit.Click
	Kontext "SolverEditSetting"
	printlog " Set time to 10 and close editdialog by OK"
	SolverNumeric.SetText "10"
	SolverEditSetting.OK
	Kontext "SolverOptions"
	printlog " Click OK on SolverOptionsdialog"
	SolverOptions.OK
	Kontext "Solver"
	printlog " Click on Solve"
	Solve.Click
	printlog " A new dialog named Solving... opens telling you about the time limit"
	sleep (15)
	printlog " After that a new dialog pops up saying 'No Solution was found. The time limit was reached.'"
	Kontext "NoSolution"
	printlog " Click the Ok button to close the No Solution dialog."
	NoSolution.OK
	printlog " Click close to leave the Solver dialog."
	Kontext "Solver"
	Solver.Close
    printlog " Close document"
    Call hCloseDocument

endcase


