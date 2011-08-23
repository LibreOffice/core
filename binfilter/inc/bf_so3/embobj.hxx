/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _EMBOBJ_HXX
#define _EMBOBJ_HXX

#include <bf_so3/persist.hxx>
#include <bf_so3/pseudo.hxx>
#include <bf_so3/protocol.hxx>
#include <bf_so3/svstor.hxx>
#include <com/sun/star/uno/Reference.hxx>
#ifndef _MAPMOD_HXX //autogen
#include <vcl/mapmod.hxx>
#endif
#include <bf_svtools/ownlist.hxx>

#ifndef INCLUDED_SO3DLLAPI_H
#include "bf_so3/so3dllapi.h"
#endif

class Printer;

/*************************************************************************/
// Datenart !!! Uebereinstimmung mit MS-Windows
#define ASPECT_CONTENT      1
#define ASPECT_THUMBNAIL    2 /* 120 * 120, 6 Farben DIB in MetaFile */
#define ASPECT_ICON         4
#define ASPECT_DOCPRINT     8
#define ASPECT_ALL      (ASPECT_CONTENT | ASPECT_THUMBNAIL | ASPECT_ICON | ASPECT_DOCPRINT)

struct TransferableObjectDescriptor;
class ImpOleObject;
class ImpViewObject;
class OutputDevice;
class Region;
class Menu;
class GDIMetaFile;
class JobSetup;
class SotStorage;
namespace com { namespace sun { namespace star { namespace datatransfer { class XTransferable; } } } }

namespace binfilter {
class SvEmbeddedClient;
class SvStorage;

class SO3_DLLPUBLIC SvEmbeddedObject : virtual public SvPersist,
                         public SvPseudoObject
{
friend class ImpOleObject;
friend class SvEmbeddedClient;
friend class SvEditObjectProtocol;
friend class ImplSvEditObjectProtocol;
friend class SvPersist; // nur wegen Sun-C40 Compiler
friend class SwWW6WrGrf; // SH benoetigt MakeMetafile
    BOOL                    bAutoSave:1,   // TRUE, bei DoOpen( FALSE ), Save
                            bAutoHatch:1;  // TRUE, bei Embed automatisch Hatch zeichnen
    Rectangle               aVisArea;      // leer == nicht gesetzt
    MapUnit                 nMapUnit;
    String                  aDocName;      // Documentname

    void			DrawHatch( OutputDevice * pDev, const Point & rPos,
                               const Size & rSize );
protected:
    SvEditObjectProtocol    aProt;           // Referenz auf das Protokoll
                    ~SvEmbeddedObject();
    virtual BOOL    Load( SvStorage * );
    virtual BOOL    Save();
    virtual BOOL    SaveAs( SvStorage * pNewStg );
    virtual void    LoadContent( SvStream & rStm, BOOL bOwner_ );
    virtual void    SaveContent( SvStream & rStm, BOOL bOwner_ );
    virtual void    Connect( BOOL bConnect );
    virtual void    Open( BOOL bOpen );
    virtual void    Embed( BOOL bEmbedded );
    virtual void    PlugIn( BOOL bPlugIn );
    virtual ErrCode Verb( long nVerbPos, ::binfilter::SvEmbeddedClient * pCallerClient,
                        Window * pWin, const Rectangle * pWorkAreaPixel );

    virtual BOOL	Close();
    void            SetMapUnit( MapUnit nUnit )
                    { nMapUnit = nUnit; }
    virtual void    Draw( OutputDevice *, const JobSetup & rSetup,
                          USHORT nAspect = ASPECT_CONTENT );
    virtual void    DocumentNameChanged( const String & rDocName );

public:
                    SvEmbeddedObject();
                    SO2_DECL_STANDARD_CLASS_DLL(SvEmbeddedObject,SOAPP)
    virtual void    FillClass( SvGlobalName * pClassName,
                               ULONG * pFormat,
                               String * pAppName,
                               String * pFullTypeName,
                               String * pShortTypeName,
                               long nFileFormat = SOFFICE_FILEFORMAT_CURRENT ) const;
    SvGlobalName    GetClassName() const
                    { return SvPseudoObject::GetClassName(); }

    void            SetAutoSave( BOOL bAutoSaveP )
                    { bAutoSave = bAutoSaveP; }
    BOOL            IsAutoSave() const { return bAutoSave; }
    virtual void    SetModified( BOOL = TRUE );

    void            SetAutoHatch( BOOL bAutoHatchP )
                    { bAutoHatch = bAutoHatchP; }
    BOOL            IsAutoHatch() const { return bAutoHatch; }

    SvEmbeddedClient * GetClient() const { return aProt.GetClient(); }

    // Konvertieren von StarObjects
    static BOOL InsertStarObject
    ( const String &rObjName, const String& rStgName,
      SvStream& rSrc, SvPersist* pPersist );
    // Einlesen eines StarObjects als Picture
    static BOOL LoadStarObjectPicture( SvStream& rSrc, GDIMetaFile& rMTF );

    static BOOL     MakeContentStream( SvStorage * pStor, const GDIMetaFile & );
    static BOOL     MakeContentStream( SotStorage * pStor, const GDIMetaFile & );

    // Attribute zum Feintuning
    virtual ULONG	GetMiscStatus() const;

    // Robuste Protokollsteuerung
    ErrCode         DoEmbed( BOOL bEmbedded );
    ErrCode         DoPlugIn( BOOL bPLugIn );
    ErrCode         DoOpen( BOOL bOpen );

    const String &  GetDocumentName() const { return aDocName; }

    SvEditObjectProtocol & GetProtocol() { return aProt; }
    const SvEditObjectProtocol & GetProtocol() const { return aProt; }

    void			SendViewChanged();
    void			ViewChanged( USHORT nAspect);

    GDIMetaFile& GetGDIMetaFile( GDIMetaFile& );
    virtual BOOL SetData( const String& rData );

    MapUnit         GetMapUnit() const { return nMapUnit; }
    //virtual Rectangle GetVisArea( USHORT nAspect = ASPECT_CONTENT ) const;
    virtual Rectangle GetVisArea( USHORT nAspect ) const;
    const Rectangle & GetVisArea() const;
    virtual void    SetVisArea( const Rectangle & rVisArea );
                    // Optimierung gegenueber SetVisArea, wenn dana GetVisArea
    const Rectangle & SetGetVisArea( const Rectangle & rVisArea );
    void            SetVisAreaSize( const Size & rVisSize );

    void            DoDraw( OutputDevice *, const Point & rObjPos,
                            const Fraction & rXScale,
                            const Fraction & rScaleY,
                            const JobSetup & rSetup, const Size& rSize,
                            USHORT nAspect = ASPECT_CONTENT );
    void            DoDraw( OutputDevice *, const Point & rObjPos,
                            const Size & rSize,
                            const JobSetup & rSetup,
                            USHORT nAspect = ASPECT_CONTENT );

    // WYSIWYG
    /* Return the printer of the document. A return of null means that the
     * document has no printer.
     * The default implementation call the parent if one exist or return null.
     */
    virtual	Printer *		GetDocumentPrinter();
    virtual OutputDevice*    GetDocumentRefDev();

    /* This Method is called if the printer in the document changed.
     * The default implementation does not call the childs.
     * @param pNewPrinter	the new printer. Only hold references
     *						to this object during the call.
     * @see	documentation in so3\source\inplace\embobj.cxx::OnDocumentPrinterChanged
     */
    virtual	void			OnDocumentPrinterChanged( Printer * pNewPrinter );

    virtual UINT32			GetViewAspect() const;
};
#ifndef SO2_DECL_SVEMBEDDEDOBJECT_DEFINED
#define SO2_DECL_SVEMBEDDEDOBJECT_DEFINED
SO2_DECL_REF(SvEmbeddedObject)
#endif
SO2_IMPL_REF(SvEmbeddedObject)

/*************************************************************************
*************************************************************************/
class SO3_DLLPUBLIC SvEmbeddedInfoObject : public SvInfoObject
{
    Rectangle       aVisArea;
    UINT32			nViewAspect;
protected:
                    ~SvEmbeddedInfoObject();
    virtual void	Assign( const SvInfoObject * pObj );
public:
                    SV_DECL_PERSIST1(SvEmbeddedObject,SvInfoObject,2)
                    SvEmbeddedInfoObject();
                    SvEmbeddedInfoObject( SvEmbeddedObject *, const String & rObjName );
                    SvEmbeddedInfoObject( const String& rObjName,
                                     const SvGlobalName& rClassName );

    virtual void    SetObj( SvPersist * );

    SvEmbeddedObject * GetEmbed() const { return &SvEmbeddedObjectRef( GetObj() ); }
    BOOL            IsLink() const;
    const Rectangle & GetVisArea() const;
    const Rectangle & GetInfoVisArea() const { return aVisArea; }
    void SetInfoVisArea( const Rectangle& r) { aVisArea = r; }

    UINT32 GetInfoViewAspect() const { return nViewAspect; }
    void SetInfoViewAspect( UINT32 nAspect ) { nViewAspect = nAspect; }
    UINT32			GetViewAspect() const;
};

}

#endif // _EMBOBJ_HXX
