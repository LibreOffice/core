/*************************************************************************
 *
 *  $RCSfile: document.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-09 17:14:48 $
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
#ifndef DOCUMENT_HXX
#define DOCUMENT_HXX

#define SMDLL   1

#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SOT_SOTREF_HXX //autogen
#include <sot/sotref.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFX_INTERNO_HXX //autogen
#include <sfx2/interno.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _SFX_OBJFAC_HXX //autogen
#include <sfx2/docfac.hxx>
#endif
#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef _FORMAT_HXX
#include "format.hxx"
#endif
#ifndef PARSE_HXX
#include "parse.hxx"
#endif
#ifndef SMMOD_HXX
#include "smmod.hxx"
#endif

class SmNode;
class SfxPrinter;
class Printer;


#ifndef SO2_DECL_SVSTORAGESTREAM_DEFINED
#define SO2_DECL_SVSTORAGESTREAM_DEFINED
SO2_DECL_REF(SvStorageStream)
#endif

#define HINT_DATACHANGED    1004

#define SM30BIDENT   ((ULONG)0x534D3033L)
#define SM30IDENT    ((ULONG)0x30334d53L)
#define SM304AIDENT  ((ULONG)0x34303330L)
#define SM30VERSION  ((ULONG)0x00010000L)
#define SM50VERSION  ((ULONG)0x00010001L)   //Unterschied zur SM30VERSION ist
                                            //der neue Border im Format.

#define FRMIDENT    ((ULONG)0x03031963L)
#define FRMVERSION  ((ULONG)0x00010001L)

/* Zugriff auf den Drucker sollte ausschliesslich ueber diese Klasse erfolgen
 * ==========================================================================
 *
 * Der Drucker kann dem Dokument oder auch dem OLE-Container gehoeren. Wenn
 * das Dokument also eine OLE-Dokument ist, so gehoert der Drucker auch
 * grundsaetzlich dem Container. Der Container arbeitet aber eventuell mit
 * einer anderen MapUnit als der Server. Der Drucker wird bezueglich des MapMode
 * im Konstruktor entsprechend eingestellt und im Destruktor wieder restauriert.
 * Das bedingt natuerlich, das diese Klasse immer nur kurze Zeit existieren darf
 * (etwa waehrend des Paints).
 * Die Kontrolle darueber ob der Drucker selbst angelegt, vom Server besorgt
 * oder dann auch NULL ist, uebernimmt die DocShell in der Methode GetPrt(),
 * fuer die der Access auch Friend der DocShell ist.
*/

class SmDocShell;
class EditEngine;
class EditEngineItemPool;

class SmPrinterAccess
{
    Printer *pPrinter;
public:
    SmPrinterAccess( SmDocShell &rDocShell );
    ~SmPrinterAccess();
    Printer *GetPrinter()   { return pPrinter; }
};


////////////////////////////////////////////////////////////

class SmDocShell : public SfxObjectShell, public SfxInPlaceObject,
                    public SfxListener
{
    friend class SmPrinterAccess;

    String              aText;
    SmFormat            aFormat;
    SmParser            aInterpreter;
    SvDataTypeList      aDataTypeList;
    SvStorageStreamRef  aDocStream;
    SmNode             *pTree;
    SvInPlaceMenuBar   *pMenuBar;
    SfxMenuBarManager  *pMenuMgr;
    SfxItemPool        *pEditEngineItemPool;
    EditEngine         *pEditEngine;
    SfxPrinter         *pPrinter;       //Siehe Kommentar zum SmPrinter Access!
    Printer            *pTmpPrinter;    //ebenfalls
    long                nLeftBorder,
                        nRightBorder,
                        nTopBorder,
                        nBottomBorder;
    USHORT              nModifyCount;
    BOOL                bIsFormulaArranged;



    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                        const SfxHint& rHint, const TypeId& rHintType);

    void        RestartFocusTimer ();

    BOOL        Try3x( SvStorage *pStor, StreamMode eMode);
    BOOL        Try2x( SvStorage *pStor, StreamMode eMode);
    BOOL        WriteAsMathType3( SfxMedium& );

    virtual void        Draw(OutputDevice *pDevice,
                             const JobSetup & rSetup,
                             USHORT nAspect = ASPECT_CONTENT);

    virtual void        FillRegInfo(SvEmbeddedRegistryInfo * pInfo);
    virtual void        FillClass(SvGlobalName* pClassName,
                                  ULONG*  pFormat,
                                  String* pAppName,
                                  String* pFullTypeName,
                                  String* pShortTypeName,
                                  long    nFileFormat = SOFFICE_FILEFORMAT_CURRENT) const;

    virtual const       SvDataTypeList& GetTypeList() const;
    virtual BOOL        GetData(SvData *pData);
    virtual BOOL        SetData(SvData *pData);
    virtual BOOL        SetData( const String& rData );
    virtual ULONG       GetMiscStatus() const;
    virtual void        OnDocumentPrinterChanged( Printer * );
    virtual BOOL        InitNew(SvStorage *);
    virtual BOOL        Load(SvStorage *);
    virtual BOOL        Insert(SvStorage *);
            void        ImplSave(  SvStorageStreamRef xStrm  );
    virtual BOOL        Save();
    virtual BOOL        SaveAs( SvStorage *pNewStor );
    virtual BOOL        ConvertTo( SfxMedium &rMedium );
    virtual BOOL        SaveCompleted( SvStorage *pNewStor );
    virtual void        HandsOff();

    Printer            *GetPrt();

    void                Convert40To50Txt();

    BOOL                IsFormulaArranged() const { return bIsFormulaArranged; }
    void                SetFormulaArranged(BOOL bVal) { bIsFormulaArranged = bVal; }
    void                ArrangeFormula();

    virtual BOOL        ConvertFrom(SfxMedium &rMedium);
            BOOL        InsertFrom(SfxMedium &rMedium);

    BOOL                ImportSM20File(SvStream *pStream, BOOL bInsert = FALSE);

public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SFX_INTERFACE_SMA_START+1);
#if 0
    SFX_DECL_OBJECTFACTORY(SmDocShell);
#else
    SFX_DECL_OBJECTFACTORY_DLL(SmDocShell, SM_MOD());
#endif

                SmDocShell(SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED);
    virtual     ~SmDocShell();

    void        LoadSymbols();
    void        SaveSymbols();

    //Zugriff fuer die View. Diese Zugriffe sind nur fuer den nicht OLE-Fall!
    //und fuer die Kommunikation mit dem SFX!
    //Alle internen Verwendungen des Printers sollten ausschlieslich uber
    //den SmPrinterAccess funktionieren.
    BOOL        HasPrinter()    { return 0 != pPrinter; }
    SfxPrinter *GetPrinter()    { GetPrt(); return pPrinter; }
    void        SetPrinter( SfxPrinter * );

    const String &GetTitle() const;
    const String &GetComment() const;

    void        SetText(const String& rBuffer);
    String&     GetText() { return (aText); }
    void        SetFormat(SmFormat& rFormat);
    SmFormat&   GetFormat() { return (aFormat); }

    void            Parse();
    SmParser &      GetParser() { return aInterpreter; }
    const SmNode *  GetFormulaTree() const  { return pTree; }
    void            SetFormulaTree(SmNode *&rTree) { pTree = rTree; }

    EditEngine &    GetEditEngine();
    SfxItemPool &   GetEditEngineItemPool();

    void        Draw(OutputDevice &rDev, Point &rPosition);
    Size        GetSize();

    void        Resize();

    virtual     SfxUndoManager *GetUndoManager ();

    virtual     SfxItemPool& GetPool();

    void        Execute( SfxRequest& rReq );
    void        GetState(SfxItemSet &);

    virtual SvDataMemberObjectRef CreateSnapshot ();
    virtual void SetVisArea (const Rectangle & rVisArea);
    virtual void UIActivate (BOOL bActivate);

    virtual void SetModified(BOOL bModified);
};


#endif

