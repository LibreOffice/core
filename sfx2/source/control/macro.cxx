/*************************************************************************
 *
 *  $RCSfile: macro.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _ARGS_HXX //autogen
#include <svtools/args.hxx>
#endif

#ifndef _SBXVAR_HXX //autogen
#include <svtools/sbxvar.hxx>
#endif
#pragma hdrstop

#include <app.hxx>
#include <module.hxx>
#include <shell.hxx>
#include <request.hxx>
#include <objsh.hxx>
#include <viewsh.hxx>
#include <viewfrm.hxx>
#include <msg.hxx>
#include <macro.hxx>
#include <sfxtypes.hxx>

//====================================================================

SV_DECL_PTRARR_DEL( SfxStatements_Impl, SfxMacroStatement*, 16, 8 );
SV_IMPL_PTRARR( SfxStatements_Impl, SfxMacroStatement* );
TYPEINIT1(SfxMacroItem,SfxPoolItem);

//--------------------------------------------------------------------

struct SfxMacro_Impl

/*  [Beschreibung]

    Implementations-Struktur der Klasse <SfxMacro>.
*/

{
    SfxMacroMode        eMode;  /*  Zweck der <SfxMacro>-Instanz,
                                    Bedeutung siehe enum <SfxMacroMode> */
    SfxStatements_Impl  aList;  /*  Liste von aufgezeichneten Statements */
    USHORT              nObjNo; //  Durchnumerierung der SbxObjects
};

//====================================================================

SfxMacroStatement::SfxMacroStatement
(
    const SfxShell& rShell,         // <SfxShell>, die den Request ausf"uhrte
    const String&   rTarget,        // Name des Zielobjektes vor der Ausf"urhung
    BOOL            bAbsolute,      // obsolet
    const SfxSlot&  rSlot,          // der <SfxSlot>, der das Statement abspielen kann
    BOOL            bRequestDone,   // wurde der Request tats"achlich ausgef"uhrt
    SfxArguments*   pArguments      // aktuelle Parameter  (werden "ubernommen)
)

/*  [Beschreibung]

    Dieser Konstruktor der Klasse SfxMacroStatement erzeugt ein Statement,
    bei dem ein Objekt angesprochen wird, welches durch 'rShell' angegeben
    ist. Dabei erfolgt die Addressierung je nach 'bAbsolute' absolut,
    also z.B. als '[mydoc.sdc]' oder relativ, also z.B. 'ActiveDocument'.

    Je nach Art der Subklasse von 'rShell' ergeben sich folgende
    Ausdr"ucke:

                          |ˇabsolut                 relativ
    -----------------------------------------------------------------------
    SfxApplication'       | 'StarCalc'              'Application'
    SfxViewFrame'         |ˇ'[mydoc.sdc:1]'         'ActiveWindow'
    SfxViewShell'         | '[mydoc.sdc:1]'         'AvtiveWindow'
    SfxObjectShell'       | '[mydoc.sdc]'           'ActiveDocument'
    sonstige (Sub-Shells) | '[mydoc.sdc:1]'         'ActiveWindow'

    Dabei sind 'StarCalc' stellvertretend fÅr den Namen der Applikation
    (Application::GetAppName()const). In der absoluten Fassung k"onnte
    die Selektion auch deskriptiv z.B. als 'CellSelection("A5-D8")')
    angesprochen werden, dazu mu\ jedoch vom Anwendungsprogrammierer der
    Konstruktor <SfxMacroStatement::SfxMacroStatement(const String&,
    const SfxSlot&,BOOL,SfxArguments*)> verwendet werden.

    F"ur das so bezeichnete Objekt wird dann je nach Typ des Slots
    eine Zuweisung an eines seiner Properties oder der Aufruf einer seiner
    Methoden ausgedr"uckt.


    [Beispiele]

    absolut:
    SCalc3.OpenDocument( "\docs\mydoc.sdd", "StarDraw Presentation", 0, 0 )
    [mydoc.sdd].Activate()
    [mydoc.sdd:1].SwitchToView( 2 )
    [mydoc.sdc:1:1].CellSelection( "A5-D8" ).LineColor = 192357

    relativ:
    ActiveWindow.LineColor = 192357


    [Querverweise]

    <SfxMacroStatement::SfxMacroStatement(const String&,const SfxSlot&,BOOL,SfxArguments*)>
    <SfxMacroStatement::SfxMacroStatement(const String&)>
*/

:   nSlotId( rSlot.GetSlotId() ),
    pArgs( pArguments ),
    bDone( bRequestDone ),
    pDummy( 0 )
{
    // Workaround Recording nicht exportierter Slots (#25386#)
    if ( !rSlot.pName )
        return;

    // Objekt-Typ bestimmen
    FASTBOOL bIsApp = rShell.ISA(SfxApplication);
    FASTBOOL bIsDoc = rShell.ISA(SfxObjectShell);
    FASTBOOL bIsWin = !bIsApp && !bIsDoc &&
                      ( rShell.ISA(SfxViewShell) || rShell.ISA(SfxViewFrame) );
    FASTBOOL bIsSel = !bIsApp && !bIsDoc && !bIsWin;

    // Objekt nicht schon im Slot-Namen enthalten?
    if ( bIsSel || rSlot.pName[0] == '.' )
    {
        // absolutes Aufzeichnen?
        if ( rSlot.IsMode( SFX_SLOT_RECORDABSOLUTE ) )
        {
            // an der Applikation oder am Modul
            if ( rShell.ISA(SfxApplication) || rShell.ISA(SfxModule) )
                aStatement = rTarget;

            // am Dokument?
            // '[' = 5Bh
            // ']' = 5Dh
            else if ( rShell.ISA(SfxObjectShell) )
            {
                aStatement = 0x005B;
                aStatement += rTarget;
                aStatement += 0x005D;
            }

            else if ( rShell.ISA(SfxViewFrame) )
            {
                aStatement = 0x005B;
                aStatement += rShell.GetSbxObject()->GetName();
                aStatement += 0x005D;
            }

            else
            {
                // an der View oder Sub-Shell
                SfxViewShell *pViewShell = rShell.GetViewShell();
                aStatement = 0x005B;
                aStatement += pViewShell->GetViewFrame()->GetSbxObject()->GetName();
                aStatement += 0x005D;
                if ( !rShell.ISA(SfxViewFrame) )
                    // an einer Sub-Shell zus"atlich Ô.SelectionÔ anh"angen
                    aStatement += DEFINE_CONST_UNICODE(".Selection");
            }
        }
        else // relatives Aufzeichnen
        {
            // an der Application?
            if ( rShell.ISA(SfxApplication) )
                aStatement = DEFINE_CONST_UNICODE("Application");

            // am Modul?
            else if ( rShell.ISA(SfxModule) )
                aStatement = DEFINE_CONST_UNICODE("ActiveModule");

            // am Dokument
            else if ( rShell.ISA(SfxObjectShell) )
                aStatement = DEFINE_CONST_UNICODE("ActiveDocument");

            // am Window
            else if ( rShell.ISA(SfxViewShell) || rShell.ISA(SfxViewFrame) )
                aStatement = DEFINE_CONST_UNICODE("ActiveWindow");

            else
                // an einer Sub-Shell
                aStatement = DEFINE_CONST_UNICODE("Selection");
        }
    }
/*
    if ( bIsSel )
    {
        // bei Selection ggf. noch den Namen der SubShell anh"angen
        const SfxShellObject *pShObj =
                    (const SfxShellObject*) rShell.GetSbxObject();
        if ( pShObj )
        {
            const SfxShellObject *pParentObj =
                        (const SfxShellObject*) pShObj->GetParent();
            SfxShell *pParentSh = pParentObj->GetShell();
            DBG_ASSERT( pParentSh->ISA(SfxViewFrame),
                        "parent of SubShell must be a Frame" );
            if ( rSlot.pName[0] == '.' )
            {
                aStatement += '.';
                aStatement += rShell.GetSbxObject()->GetName();
            }
        }
        else
            DBG_ASSERT( rSlot.pName[0] != '0', "recording unnamed object" );
    }
*/
    // an diesen Objekt-Ausdruck den Methoden-/Property-Namen und Parameter
    GenerateNameAndArgs_Impl( SfxRequest::GetRecordingMacro(),
                              rSlot, bRequestDone, pArgs);
}

//--------------------------------------------------------------------

SfxMacroStatement::SfxMacroStatement
(
    const String&   rTarget,        // Objekt, was beim Playing angesprochen wird
    const SfxSlot&  rSlot,          // der <SfxSlot>, der das Statement abspielen kann
    BOOL            bRequestDone,   // wurde der Request tats"achlich ausgef"uhrt
    SfxArguments*   pArguments      // aktuelle Parameter (werden "ubernommen)
)

/*  [Beschreibung]


    [Querverweise]

    <SfxMacroStatement::SfxMacroStatement(const String&)>
    <SfxMacroStatement::SfxMacroStatement(const SfxShell&,BOOL,const SfxSlot&,BOOL,SfxArguments*)>
*/

:   nSlotId( rSlot.GetSlotId() ),
    pArgs( pArguments ),
    bDone( bRequestDone ),
    pDummy( 0 )
{
    aStatement = rTarget;
    aStatement += 0x002E; // '.' = 2Eh
    GenerateNameAndArgs_Impl( SfxRequest::GetRecordingMacro(),
            rSlot, bRequestDone, pArgs);
}

//--------------------------------------------------------------------

SfxMacroStatement::SfxMacroStatement
(
    const String&   rStatement      // manuell erzeugte(s) Statement(s)
)

/*  [Beschreibung]

    Dieser Konstruktor erzeugt eine SfxMacroStatement-Instanz, deren
    Aufbau vollst"andig vom Applikationsentwickler bestimmt wird. Da der
    angegebene String auch mehrere mit CR/LF getrennte Statements
    enthalten darf, ist damit ein weitgehender Eingriff in das Aufzeichnen
    von BASIC-Makros m"oglich, um Spezialf"alle zu behandeln.


    [Querverweise]

    <SfxMacroStatement::SfxMacroStatement(const String&,const SfxSlot&,BOOL,SfxArguments*)>
    <SfxMacroStatement::SfxMacroStatement(const SfxShell&,BOOL,const SfxSlot&,BOOL,SfxArguments*)>
*/

:   aStatement( rStatement ),
    nSlotId( 0 ),
    pArgs( 0 ),
    bDone( TRUE ),
    pDummy( 0 )
{
}

//--------------------------------------------------------------------

SfxMacroStatement::SfxMacroStatement
(
    const SfxMacroStatement&    rOrig // Original, von dem kopiert wird
)

/*  [Beschreibung]

    Copy-Konstruktor der SfxMacroStatement-Klasse.
*/

:   aStatement( rOrig.aStatement ),
    nSlotId( rOrig.nSlotId ),
    pArgs( 0 ),
    bDone( rOrig.bDone ),
    pDummy( 0 )
{
    if ( rOrig.pArgs )
        pArgs = new SfxArguments( *rOrig.pArgs );
}

//--------------------------------------------------------------------

SfxMacroStatement::~SfxMacroStatement()

/*  [Beschreibung]

    Destruktor der Klasse SfxMacroStatement. Gibt die Liste der
    aktuellen Parameter frei.
*/

{
    delete pArgs;
}

//--------------------------------------------------------------------

void SfxMacroStatement::GenerateNameAndArgs_Impl
(
    SfxMacro*       pMacro,         // darin wird aufgezeichnet
    const SfxSlot&  rSlot,          // der Slot, der das Statement abspielen kann
    BOOL            bRequestDone,   // TRUE=wurde ausgef"uhrt, FALSE=abgebrochen
    SfxArguments*   pArgs           // die aktuellen Parameter
)

/*  [Beschreibung]

    Interne Hilfsmethode zum generieren des Funktions-/Property-Names
    sowie der Parameter. Diese Methode wird nur verwendet, wenn der
    Anwendungsprogrammierer keinen eigenen Source an den <SfxRequest>
    geh"angt hat.
*/

{
    // Ô.Ô zwsischen Object und Prop/Meth-Name
    // '.' = 2Eh
    if ( aStatement.Len() && aStatement.GetChar( aStatement.Len() - 1 ) != 0x002E &&
        rSlot.pName[0] != 0x002E )
        aStatement += 0x002E;

    // der Name des Slots ist der Name der Methode / des Properties
    aStatement += String::CreateFromAscii(rSlot.pName);
    if ( rSlot.IsMode(SFX_SLOT_METHOD) )
        aStatement += DEFINE_CONST_UNICODE("( ");
    else
        aStatement += DEFINE_CONST_UNICODE(" = ");

    // alle zusammengesuchten Parameter rausschreiben
    if ( pArgs && pArgs->Count() ) // Abfrage doppelt wegen BLC-Bug
        for ( USHORT nArg = 0; nArg < pArgs->Count(); ++nArg )
        {
            // den Parameter textuell darstellen
            String aArg;
            const SbxVariable& rVar = pArgs->Get(nArg);
            switch ( rVar.GetType() & (2*SbxUSERn+1) )
            {
                case SbxEMPTY:
                case SbxNULL:
                    // kein Argument
                    break;

                case SbxSTRING:
                {
                    // Anf"uhrungszeichen werden verdoppelt
                    XubString aRecordable( rVar.GetString() );
                    USHORT nPos = 0;
                    while ( TRUE )
                    {
                        nPos = aRecordable.SearchAndReplace( DEFINE_CONST_UNICODE('"'), DEFINE_CONST_UNICODE("\"\""), nPos );
                        if ( STRING_NOTFOUND == nPos )
                            break;
                        nPos += 2;
                    }

                    // nicht druckbare Zeichen werden als chr$(...) geschrieben
                    FASTBOOL bPrevReplaced = FALSE;
                    for ( USHORT n = 0; n < aRecordable.Len(); ++n )
                    {
                        sal_Unicode cChar = aRecordable.GetChar(n);
                        if ( !( cChar>=32 && cChar!=127 ) ) // ALS ERSATZ FUER String::IsPrintable()!
                        {
                            XubString aReplacement( DEFINE_CONST_UNICODE("+chr$(") );
                            aReplacement += cChar;

                            if ( bPrevReplaced )
                            {
                                aRecordable.Insert( aReplacement, n - 2 );
                                n += aReplacement.Len();
                                aRecordable.SetChar((unsigned short) (n-2), 0x0029);// ')' = 29h
                                aRecordable.Replace( n-1, 2, DEFINE_CONST_UNICODE("+\"") );
                                // ++n;
                            }
                            else
                            {
                                aReplacement += DEFINE_CONST_UNICODE(")+\"");
                                aRecordable.SetChar(n, 0x0022 );// '"' = 22h
                                aRecordable.Insert( aReplacement, n + 1 );
                                n += aReplacement.Len();
                            }
                            bPrevReplaced = TRUE;
                        }
                        else
                            bPrevReplaced = FALSE;
                    }

                    // Argument in Anf"uhrungszeichen
                    aArg = 0x0022; // '"' = 22h
                    aArg += aRecordable;
                    aArg += 0x0022;
                    break;
                }

                case SbxBOOL:
                {
                    // symbolisch als TRUE und FALSE
                    aArg = rVar.GetBool() ? DEFINE_CONST_UNICODE("TRUE") : DEFINE_CONST_UNICODE("FALSE");
                    break;
                }

                case SbxBYTE:
                {
                    // als Zahl darstellen
                    aArg = (USHORT) rVar.GetByte();
                    break;
                }

                case SbxOBJECT:
                {
                    HACK(only an experimental implementation)
                    SbxObjectRef xObj = (SbxObject*) rVar.GetObject();

                    // einen Objektnamen generieren
                    String aObjectName( 'a' );
                    aObjectName += xObj->GetClassName();
                    aObjectName += pMacro->NextObjectNo();

                    // das Objekt mit Daten belegen
                    String aObjectDef( DEFINE_CONST_UNICODE("\nDIM ") );
                    aObjectDef += aObjectName;
                    aObjectDef += DEFINE_CONST_UNICODE(" AS NEW ");
                    aObjectDef += xObj->GetClassName();
                    aObjectDef += DEFINE_CONST_UNICODE("\nWITH ");
                    aObjectDef += aObjectName;
                    aObjectDef += DEFINE_CONST_UNICODE("\n");
                    aObjectDef += xObj->GenerateSource( DEFINE_CONST_UNICODE("\t"), 0 );
                    aObjectDef += DEFINE_CONST_UNICODE("\nEND WITH\n");
                    aStatement.Insert( aObjectDef, 0 );
                    aObjectDef += DEFINE_CONST_UNICODE("\n\n");

                    // Objektname als Parameter anh"angen
                    aArg = aObjectName;
                    break;
                }

                default:
                    // alles andere als String ohne Anf"uhrungszeichen
                    aArg = rVar.GetString();
            }

            // den Parameter anh"angen
            aStatement += aArg;
            aStatement += DEFINE_CONST_UNICODE(", ");
        }

    // Statement beeden
    if ( pArgs && pArgs->Count() )
        aStatement.Erase( aStatement.Len() - 2, 1 );
    else
        aStatement.Erase( aStatement.Len() - 1, 1 );
    if ( rSlot.IsMode(SFX_SLOT_METHOD) )
        aStatement += 0x0029; // ')' = 29h

    if ( !bRequestDone )
        // nicht als "Done()" gekennzeichnete Statements auskommentieren
        aStatement.InsertAscii( "rem ", 0 );
}

//--------------------------------------------------------------------

SfxMacro::SfxMacro
(
    SfxMacroMode    eMode       // Zweck der Instanz, siehe <SfxMacroMode>
)

/*  [Beschreibung]

    Konstruktor der Klasse SfxMacro. Instanzen dieser Klasse werden im
    SFx zu zwei Zwecken ben"otigt:

    1. zum Aufzeichnen von Makros
    In diesem Fall wird der Konstruktor mit SFX_MACRO_RECORDINGABSOLUTE
    oder SFX_MACRO_RECORDINGRELATIVE aufgerufen. Es sollte sich um eine
    Instanz einer abgeleiteten Klasse handeln, um in der Ableitung
    die Information dar"uber unterzubringen, wo das Makro gespeichert
    werden soll. Ein solches Makro solle sich dann in seinem Destruktor
    an der vom Benutzer gew"ahlten Stelle speichern.

    2. zum Zuordnen von exisitierenden Makros
    In diesem Fall wird der Konstruktor mit SFX_MACRO_EXISTING aufgerufen.
    Eine solche Instanz wird z.B. ben"otigt, wenn Makros auf Events
    oder <SfxControllerItem>s konfiguriert werden sollen.
*/

:   pImp( new SfxMacro_Impl )

{
    pImp->eMode = eMode;
    pImp->nObjNo = 0;
}

//--------------------------------------------------------------------

SfxMacro::~SfxMacro()

/*  [Beschreibung]

    Virtueller Destruktor der Klasse SfxMacro. Dieser sollte in den
    abgeleiteten Klassen "uberladen werden, um in den Modi
    SFX_MACRO_RECORDINGABSOLUTE und SFX_MACRO_RECORDINGRELATIVE den
    aufgezeichneten Source abzuspeichern.


    [Querverweise]

    <SfxMacro::GenerateSource()const>
*/

{
#ifdef DBG_UTIL
#ifdef MAC
    SvFileStream aStream( DEFINE_CONST_UNICODE("record.bas"), STREAM_STD_WRITE );
#else
    SvFileStream aStream( DEFINE_CONST_UNICODE("/tmp/record.bas"), STREAM_STD_WRITE );
#endif
    aStream.WriteLine( ByteString(U2S(GenerateSource())) );
#endif
    delete pImp;
}

//--------------------------------------------------------------------

SfxMacroMode SfxMacro::GetMode() const

/*  [Beschreibung]

    Liefert den Modus, der besagt zu welchem Zweck das SfxMacro konstruiert
    wurde.


    [Querverweise]

    enum <SfxMacroMode>
*/

{
    return pImp->eMode;
}

//--------------------------------------------------------------------

USHORT SfxMacro::NextObjectNo()

/*  [Beschreibung]

    Liefert eine Nummer, die als Namens-Erweiterung f"ur SbxObjects,
    die per DIM angelegt werden m"ussen, verwendet werden kann. Bei jedem
    Abfragen kommt eine um 1 erh"ohte Nummer zur"uck, die in diesem Makro
    einmalig ist.
*/

{
    return ++pImp->nObjNo;
}

//--------------------------------------------------------------------

void SfxMacro::Record
(
    SfxMacroStatement*  pStatement  // aufzuzeichnendes <SfxMacroStatement>
)

/*  [Beschreibung]

    Diese Methode zeichnet das als Parameter "ubergeben Statement auf.
    Die Instanz auf die der "ubergebe Pointer zeigt, geht in das Eigentum
    des SfxMacro "uber.

    Der Aufruf ist nur g"ultig, wenn es sich um ein SfxMacro handelt,
    welches mit SFX_MACRO_RECORDINGABSOLUTE oder SFX_MACRO_RECORDINGRELATIVE
    konstruiert wirde.


    [Querverweise]

    <SfxMacro::Replace(SfxMacroStatement*)>
    <SfxMacro::Remove()>
    <SfxMacro::GetLastStatement()const>
*/

{
    DBG_ASSERT( pImp->eMode != SFX_MACRO_EXISTING,
                "invalid call to non-recording SfxMacro" );
    pImp->aList.C40_INSERT( SfxMacroStatement, pStatement, pImp->aList.Count() );
}

//--------------------------------------------------------------------

void SfxMacro::Replace
(
    SfxMacroStatement*  pStatement  // aufzuzeichnendes <SfxMacroStatement>
)

/*  [Beschreibung]

    Diese Methode zeichnet das als Parameter "ubergeben Statement auf.
    Dabei wird das jeweils zuletzt aufgezeichnete Statement "uberschrieben.
    Die Instanz auf die der "ubergebe Pointer zeigt, geht in das Eigentum
    des SfxMacro "uber.

    Mit dieser Methode ist es m"oglich, Statements zusammenzufassen. Z.B.
    anstelle f"unfmal hintereinander 'CursorLeft()' aufzurufen, k"onnte
    das zu 'CursorLeft(5)' zusammengefa\st werden. Oder anstelle ein Wort
    Buchstabe f"ur Buchstabe aufzubauen, k"onnte dies durch ein einziges
    Statement 'InsertString("Hallo")' ersetzt werden.

    Der Aufruf ist nur g"ultig, wenn es sich um ein SfxMacro handelt,
    welches mit SFX_MACRO_RECORDINGABSOLUTE oder SFX_MACRO_RECORDINGRELATIVE
    konstruiert wurde und bereits ein aufgezeichnetes Statement vorhanden
    ist.


    [Anmerkung]

    Diese Methode wird typischerweise aus den Execute-Methoden der
    <SfxSlot>s von den Applikationsentwicklern gerufen.


    [Querverweise]

    <SfxMacro::Record(SfxMacroStatement*)>
    <SfxMacro::Remove()>
    <SfxMacro::GetLastStatement()const>
*/

{
    DBG_ASSERT( pImp->eMode != SFX_MACRO_EXISTING,
                "invalid call to non-recording SfxMacro" );
    DBG_ASSERT( pImp->aList.Count(), "no replaceable statement available" )
    pImp->aList.Remove( pImp->aList.Count() - 1 );
    pImp->aList.C40_INSERT( SfxMacroStatement,pStatement, pImp->aList.Count() );
}

//--------------------------------------------------------------------

void SfxMacro::Remove()

/*  [Beschreibung]

    Diese Methode l"oscht das zuletzt aufgezeichnete <SfxMacroStatement>
    und entfernt es aus dem Macro.

    Mit dieser Methode ist es m"oglich, Statements zusammenzufassen. Z.B.
    anstelle f"unfmal hintereinander 'CursorLeft()' aufzurufen, k"onnte
    das zu 'CursorLeft(5)' zusammengefa\st werden. Oder anstelle ein Wort
    Buchstabe f"ur Buchstabe aufzubauen, k"onnte dies durch ein einziges
    Statement 'InsertString("Hallo")' ersetzt werden.

    Der Aufruf ist nur g"ultig, wenn es sich um ein SfxMacro handelt,
    welches mit SFX_MACRO_RECORDINGABSOLUTE oder SFX_MACRO_RECORDINGRELATIVE
    konstruiert wurde und bereits ein aufgezeichnetes Statement vorhanden
    ist.


    [Anmerkung]

    Diese Methode wird typischerweise aus den Execute-Methoden der
    <SfxSlot>s von den Applikationsentwicklern gerufen.


    [Querverweise]

    <SfxMacro::Replace(SfxMacroStatement*)>
    <SfxMacro::Record(SfxMacroStatement*)>
    <SfxMacro::GetLastStatement()const>
*/

{
    DBG_ASSERT( pImp->eMode != SFX_MACRO_EXISTING,
                "invalid call to non-recording SfxMacro" );
    DBG_ASSERT( pImp->aList.Count(), "no replaceable statement available" )
    pImp->aList.Remove( pImp->aList.Count() - 1 );
}

//--------------------------------------------------------------------

const SfxMacroStatement* SfxMacro::GetLastStatement() const

/*  [Beschreibung]

    Mit dieser Methode kann auf das jeweils zuletzt aufgezeichnete Statement
    lesend zugegriffen werden. Zusammen mit der Methode
    <SfxMacro::Replace(SfxMacroStatement*)> ergibt sich dadurch die
    M"oglichkeit, Statements zusammenzufassen.

    Der Aufruf ist nur g"ultig, wenn es sich um ein SfxMacro handelt,
    welches mit SFX_MACRO_RECORDINGABSOLUTE oder SFX_MACRO_RECORDINGRELATIVE
    konstruiert wurde.


    [Querverweise]

    <SfxMacro::Record(SfxMacroStatement*)>
    <SfxMacro::Replace(SfxMacroStatement*)>
*/

{
    DBG_ASSERT( pImp->eMode != SFX_MACRO_EXISTING,
                "invalid call to non-recording SfxMacro" );
    if ( pImp->aList.Count() )
        return pImp->aList.GetObject( pImp->aList.Count() - 1 );
    return 0;
}

//--------------------------------------------------------------------

String SfxMacro::GenerateSource() const

/*  [Beschreibung]

    Diese Funktion generiert aus den, seit dem Konstruieren der Instanz
    bis zum Zeitpunkt des Aufrufs dieser Methode aufgezeichneten
    <SfxMacroStatement>s einen BASIC-Sourcecode, der die Statements,
    jedoch nicht den Header ('Sub X') und den Footer ('End Sub') enth"alt.


    [Querverweise]

    <SfxMacro::Record(SfxMacroStatement*)>
    <SfxMacro::Repeat(SfxMacroStatement*)>
*/

{
    DBG_ASSERT( pImp->eMode != SFX_MACRO_EXISTING,
                "invalid call to non-recording SfxMacro" );
    String aSource;
    for ( USHORT n = 0; n < pImp->aList.Count(); ++n )
    {
        aSource += pImp->aList.GetObject(n)->GetStatement();
        if ( (n+1) < pImp->aList.Count() )
            aSource += DEFINE_CONST_UNICODE("\n");
    }

    return aSource;
}

//--------------------------------------------------------------------

SbxError SfxMacro::Call
(
    SbxArray*   pArgs           // aktuelle Parameter f"ur den Aufruf
)

/*  [Beschreibung]

    Diese Methode sollte von Suklassen "uberladen werden, damit der
    SFx das in der Ableitung beschriebene BASIC-Programm rufen kann,
    wenn es an ein Event oder in einem Controller gebunden ist.

    Der Aufruf ist nur g"ultig, wenn es sich um ein SfxMacro handelt,
    welches mit SFX_MACRO_RECORDINGABSOLUTE oder SFX_MACRO_RECORDINGRELATIVE
    konstruiert wurde.

    Die Basisimplementierung liefert immer SbxERR_NOTIMP zur"uck.


    [Anmerkung]

    I.d.R. ist pArgs == 0, der Parameter ist f"ur Zuk"unftige Erweiterungen
    gedacht.
*/

{
    return SbxERR_NOTIMP;
}

//--------------------------------------------------------------------

SfxMacroItem::SfxMacroItem( USHORT nWhich, SfxMacro *pTheMacro )
:   SfxPoolItem( nWhich ),
    pMacro( pTheMacro )
{
}

//--------------------------------------------------------------------

SfxPoolItem* SfxMacroItem::Clone( SfxItemPool *pPool ) const
{
    return new SfxMacroItem( Which(), pMacro );
}

//--------------------------------------------------------------------

int SfxMacroItem::operator==( const SfxPoolItem &rOther ) const
{
    return pMacro == ((const SfxMacroItem&)rOther).pMacro;
}


