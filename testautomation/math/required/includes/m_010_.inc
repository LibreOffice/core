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
'* Owner : thorsten.bosbach@oracle.com
'*
'* short description :
'*
'\*****************************************************************
'/// If a function button is not visible, the test exists. ///'

sub M_010_

   printlog Chr(13) +  "--  Operators  --"

   Call tAllUnaerBinaer
   Call tAllRelations
   Call tAllMixOperators
   Call tAllFunctions
   Call tAllOperators
   Call tAllAttributes
   Call tAllBrackets
   Call tAllFormats
   Call tOthers

end sub


'-----------------------------------------------------------
testcase tAllUnaerBinaer
   PrintLog "- SelectionWindow : All UnaerBinaer"
   Call hNewDocument

  Kontext "AuswahlMath"
  If AuswahlMath.notexists(2) then
   ViewSelection
  endif
  ' others don't work ?? OperatorenMAth, SelectionMAth, ViewSelection...
  Kontext "OperatorenMath"

   UnaereBinaere.Click
   Sleep 1
  Kontext "AuswahlMath"
  Call DialogTest ( AuswahlMath, 1 )

try
   Plus.Click
    Call Bereinigen
   Minus.Click
    Call Bereinigen
   PlusMinus.Click
    Call Bereinigen
   MinusPlus.Click
    Call Bereinigen
   Additions.Click
    Call Bereinigen
   Multiplikation.Click
    Call Bereinigen
   MultiplikationKreuz.Click
    Call Bereinigen
   MultiplikationStern.Click
    Call Bereinigen
   Subtraktions.Click
    Call Bereinigen
   Division.Click
    Call Bereinigen
   DivisionDoppelpunkt.Click
    Call Bereinigen
   DivisionSchraegstrich.Click
    Call Bereinigen
   LogischesNICHT.Click
    Call Bereinigen
   LogischesUND.Click
    Call Bereinigen
   LogischesODER.Click
       Call Bereinigen
   Verkettung.Click
catch
   warnlog "failed!"
endcatch

   Call hCloseDocument
endcase
'-----------------------------------------------------------
testcase tAllRelations
   PrintLog "- SelectionWindow : All Relations"
   Call hNewDocument

  Kontext "AuswahlMath"
  If AuswahlMath.notexists(2) then AuswahlMath
   Relationen.Click
   Sleep 1
  Call DialogTest ( AuswahlMath, 2 )

try
   IstGleich.Click
    Call Bereinigen
   IstUngleich.Click
    Call Bereinigen
   IstKongruentZu.Click
    Call Bereinigen
   IstOrthogonalZu.Click
    Call Bereinigen
   IstKleinerAls.Click
    Call Bereinigen
   IstGroesserAls.Click
    Call Bereinigen
   IstUngefaehrGleich.Click
    Call Bereinigen
   IstParallelZu.Click
    Call Bereinigen
   IstKleinerGleich.Click
    Call Bereinigen
   IstGroesserGleich.Click
    Call Bereinigen
   IstAehnlichOderGlech.Click
    Call Bereinigen
   IstProportionalZu.Click
    Call Bereinigen
   IstKleinerGleich2.Click
    Call Bereinigen
   IstGroesserGleich2.Click
    Call Bereinigen
   IstAehnlichZu.Click
    Call Bereinigen
   StrebtGegen.Click
       Call Bereinigen
   DoppelterPfeilNachLinks.Click
       Call Bereinigen
   DoppelterPfeilNachLinksUndRechts.Click
       Call Bereinigen
   DoppelterPfeilNachRechts.Click
       Call Bereinigen
   Teilt.Click
       Call Bereinigen
   TeiltNicht.Click

catch
   warnlog "failed!"
endcatch

   Call hCloseDocument
endcase
'-----------------------------------------------------------
testcase tAllMixOperators
   PrintLog "- SelectionWindow : All SetOperators"
   Call hNewDocument

  Kontext "AuswahlMath"
  If AuswahlMath.notexists(2) then AuswahlMath
   MengenOperatoren.Click
   Sleep 1
  Call DialogTest ( AuswahlMath, 3 )

try
   IstEnthaltenIn.Click
    Call Bereinigen
   IstNichtEnthaltenIn.Click
    Call Bereinigen
   Enthaelt.Click
    Call Bereinigen
   Durchschnitt.Click
    Call Bereinigen
   Vereinigung.Click
    Call Bereinigen
   Differenz.Click
    Call Bereinigen
   Quotientenmenge.Click
    Call Bereinigen
   Teilmenge.Click
    Call Bereinigen
   TeilmengeOderGleich.Click
    Call Bereinigen
   Obermenge.Click
    Call Bereinigen
   ObermengeOderGleich.Click
    Call Bereinigen
   NichtTeilmenge.Click
    Call Bereinigen
   NichtTeilmengeOderGleich.Click
    Call Bereinigen
   NichtObermenge.Click
    Call Bereinigen
   NichtObermengeOderGleich.Click
    Call Bereinigen
   Aleph.Click
       Call Bereinigen
   LeereMenge.Click
       Call Bereinigen
   MengeDerGanzenZahlen.Click
       Call Bereinigen
   MengeDerKomplexenZahlen.Click
       Call Bereinigen
   MengeDerNatuerlichenZahlen.Click
       Call Bereinigen
   MengeDerRationalenZahlen.Click
       Call Bereinigen
   MengeDerReellenZahlen.Click

catch
   warnlog "failed!"
endcatch

   Call hCloseDocument
endcase
'-----------------------------------------------------------
testcase tAllFunctions
   PrintLog "- OperatorWindow AllFunctions"
   Call hNewDocument

  Kontext "AuswahlMath"
  If AuswahlMath.notexists(2) then AuswahlMath
   Funktionen.Click
   Sleep 1
  Call DialogTest ( AuswahlMath, 4 )

try
   Absolutwert.Click
    Call Bereinigen
   Fakultaet.Click
    Call Bereinigen
   Quadratwurzel.Click
    Call Bereinigen
   NteWurzel.Click
    Call Bereinigen
   NatuerlicheExponentialfunktion.Click
    Call Bereinigen
   NatuerlicherLogarithmus.Click
    Call Bereinigen
   Exponentialfunktion.Click
    Call Bereinigen
   Logarithmus.Click
    Call Bereinigen
   Sinus.Click
    Call Bereinigen
   Cosinus.Click
    Call Bereinigen
   Tangens.Click
    Call Bereinigen
   Cotangens.Click
    Call Bereinigen
   Hyperbelsinus.Click
    Call Bereinigen
   Hyperbelcosinus.Click
    Call Bereinigen
   Hyperbeltangens.Click
    Call Bereinigen
   Hyperbelcotangens.Click
    Call Bereinigen
   Arcussinus.Click
    Call Bereinigen
   Arcuscosinus.Click
    Call Bereinigen
   Arcustangens.Click
    Call Bereinigen
   Arcuscotangens.Click
    Call Bereinigen
   Areahyperbelsinus.Click
    Call Bereinigen
   Areahyperbelcosinus.Click
    Call Bereinigen
   Areahyperbeltangens.Click
    Call Bereinigen
   Areahyperbelcotangens.Click
       Call Bereinigen
   SuperscriptRechts.Click

catch
   warnlog "failed!"
endcatch

   Call hCloseDocument
endcase
'-----------------------------------------------------------
testcase tAllOperators
   PrintLog "- Auwahlfenster : Operators"
   Call hNewDocument

  Kontext "AuswahlMath"
  If AuswahlMath.notexists(2) then AuswahlMath
   Operatoren.Click
   Sleep 1
  Call DialogTest ( AuswahlMath, 5 )

try
   Limes.Click
    Call Bereinigen
   Summe.Click
    Call Bereinigen
   Produkt.Click
    Call Bereinigen
   Coprodukt.Click
    Call Bereinigen
   Integral.Click
    Call Bereinigen
   DoppeltesIntegral.Click
    Call Bereinigen
   DreifachesIntegral.Click
    Call Bereinigen
   'EsExistiertEin.Click
   ' Call Bereinigen
   Kurvenintegral.Click
    Call Bereinigen
   DoppeltesKurvenintegral.Click
    Call Bereinigen
   DreifachesKurvenintegral.Click
    Call Bereinigen
   'FuerAlle.Click
   ' Call Bereinigen
   UntereUndObereGrenze.Click
    Call Bereinigen
   UntereGrenze.Click
    Call Bereinigen
   ObereGrenze.Click

catch
   warnlog "failed!"
endcatch

   Call hCloseDocument
endcase

'-----------------------------------------------------------
testcase tAllAttributes
   PrintLog "- SelectionWindow : All Attributes"
   Call hNewDocument

  Kontext "AuswahlMath"
  If AuswahlMath.notexists(2) then AuswahlMath
   Attribute.Click
   Sleep 1
  Call DialogTest ( AuswahlMath, 6 )

try
   AccentNachRechts.Click
    Call Bereinigen
   AccentNachLinks.Click
    Call Bereinigen
   UmgekehrtesDach.Click
    Call Bereinigen
   Breve.Click
    Call Bereinigen
   Ueberstrich.Click
    Call Bereinigen
   Vektorpfeil.Click
    Call Bereinigen
   Dach.Click
    Call Bereinigen
   Tilde.Click
    Call Bereinigen
   Kreis.Click
    Call Bereinigen
   Punkt.Click
    Call Bereinigen
   DoppelterPunkt.Click
    Call Bereinigen
   DreifacherPunkt.Click
    Call Bereinigen
   LinieUeber.Click
    Call Bereinigen
   LinieUnter.Click
    Call Bereinigen
   LinieDurch.Click
    Call Bereinigen
   Transparent.Click
    Call Bereinigen
   Fettschrift.Click
    Call Bereinigen
   KursiveSchrift.Click
    Call Bereinigen
   GroesseAendern.Click
    Call Bereinigen
   FontAendern.Click
       Call Bereinigen
   BreiterVektorpfeil.Click
       Call Bereinigen
   BreitesDach.Click
       Call Bereinigen
   BreiteTilde.Click

catch
   warnlog "failed!"
endcatch

   Call hCloseDocument
endcase

'-----------------------------------------------------------
testcase tAllBrackets
   PrintLog "- SelectionWindow : All Bracketings"
   Call hNewDocument

  Kontext "AuswahlMath"
  If AuswahlMath.notexists(2) then AuswahlMath
   Klammerungen.Click
   Sleep 1
  Call DialogTest ( AuswahlMath, 7 )

try
   RundeKlammern.Click
    Call Bereinigen
   EckigeKlammern.Click
    Call Bereinigen
   SpitzeKlammern.Click
    Call Bereinigen
   GeschweifteKlammern.Click
    Call Bereinigen
   EinfacheLinien.Click
    Call Bereinigen
   DoppelteLinien.Click
    Call Bereinigen
   GruppierungsKlammern.Click
    Call Bereinigen
   RundeKlammernSkalierbar.Click
    Call Bereinigen
   EckigeKlammernSkalierbar.Click
    Call Bereinigen
   SpitzeKlammernSkalierbar.Click
    Call Bereinigen
   GeschweifteKlammernSkalierbar.Click
    Call Bereinigen
   EinfacheLinienSkalierbar.Click
    Call Bereinigen
   DoppelteLinienSkalierbar.Click
       Call Bereinigen
   DoppelteEckigeKlammern.Click
       Call Bereinigen
   EckigeKlammernSkalierbar2.Click
       Call Bereinigen
   GeschweifteKlammerObenSkalierbar.Click
       Call Bereinigen
   GeschweifteKlammerUntenSkalierbar.Click
       Call Bereinigen
   OperatorKlammern.Click
       Call Bereinigen
   OperatorKlammernSkalierbar.Click

catch
   warnlog "failed!"
endcatch

   Call hCloseDocument
endcase

'-----------------------------------------------------------
testcase tAllFormats
   dim i as integer

   PrintLog "- SelectionWindow : All Formattings"
   Call hNewDocument

  Kontext "AuswahlMath"
  If AuswahlMath.notexists(2) then AuswahlMath
   Formatierungen.Click
   Sleep 1
  Call DialogTest ( AuswahlMath, 8 )

   i = 0
try
   NeueZeileBeginnen.Click
   inc i
   Printlog " i: "+i
    Call Bereinigen
   KleinerZwischenraum.Click
   inc i
   Printlog " i: "+i
    Call Bereinigen
   Zwischenraum.Click
   inc i
   Printlog " i: "+i
    Call Bereinigen
   SubscriptRechts.Click
   inc i
   Printlog " i: "+i
    Call Bereinigen
   VertikaleAnordnung2Elemente.Click
   inc i
   Printlog " i: "+i
    Call Bereinigen
   VertikaleAnordnung.Click
   inc i
   Printlog " i: "+i
    Call Bereinigen
   MatrixAnordnung.Click
   inc i
   Printlog " i: "+i
    Call Bereinigen
   SuperscriptRechts.Click
   inc i
   Printlog " i: "+i
    Call Bereinigen
   LinksbuendigAusrichten.Click
   inc i
   Printlog " i: "+i
    Call Bereinigen
   ZentriertHorizontalAusrichten.Click
   inc i
   Printlog " i: "+i
    Call Bereinigen
   RechtsbuendigAusrichten.Click
   inc i
   Printlog " i: "+i
    Call Bereinigen
   SubscriptLinks.Click
   inc i
   Printlog " i: "+i
    Call Bereinigen
   'NachObenAusrichten.Click
   ' Call Bereinigen
   'ZentriertVertikalAusrichten.Click
   ' Call Bereinigen
'   NachUntenAusrichten.Click
   ' Call Bereinigen
   SuperscriptLinks.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   SuperscriptOben.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   SuperscriptRechts.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   SubscriptUnten.Click
catch
   warnlog "failed!"
endcatch

   Call hCloseDocument
endcase

testcase tOthers
   dim i as integer

   PrintLog "- SelectionWindow : Others"
   Call hNewDocument

  Kontext "SelectionMath"
  If SelectionMath.notexists(2) then
    ViewSelection
  endif
   Others.Click
   Sleep 1
  Kontext "AuswahlMath"
  Call DialogTest ( AuswahlMath, 9 )
  Kontext "SelectionMath"

   i = 0
try
' there might be a problem with the new group .... :TB_Gruppen Sonstiges
' Nope, its the same as Formatierungen...

   ThereExists.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   ForAll.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   HBar.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   Kontext "AuswahlMath"
   Imaginaerteil.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   LambdaQuer.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   Nabla.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   Partial.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   PfeilNachLinks.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   PfeilNachOben.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   PfeilNachRechts.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   PfeilNachUnten.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   PunkteMittig.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   PunkteNachOben.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   PunkteUnten.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   PunkteVertikal.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   Realteil.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   Unendlich.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   WeierstrassP.Click
   inc i
   Printlog " i: "+i
       Call Bereinigen
   PunkteNachUnten.Click
   inc i
   Printlog " i: "+i
catch
   warnlog "failed!"
endcatch

   Call hCloseDocument
endcase

sub Bereinigen
   sleep 2
'   ViewUpdate
'   Sleep 2
'   EditSelectAllMath
'   Wait 500
'   EditCut
   Sleep 1
end sub

