/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tablink.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-22 10:44:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_TABLINK_HXX
#define SC_TABLINK_HXX

#ifndef INCLUDED_SCDLLAPI_H
#include "scdllapi.h"
#endif

#ifndef SC_REFRESHTIMER_HXX
#include "refreshtimer.hxx"
#endif

//REMOVE    #ifndef _EMBOBJ_HXX
//REMOVE    #include <so3/embobj.hxx>
//REMOVE    #endif

#ifndef _LNKBASE_HXX //autogen
#include <sfx2/lnkbase.hxx>
#endif

#include <sfx2/objsh.hxx>
//REMOVE    #ifndef SO2_DECL_SVEMBEDDEDOBJECT_DEFINED
//REMOVE    #define SO2_DECL_SVEMBEDDEDOBJECT_DEFINED
//REMOVE    SO2_DECL_REF(SvEmbeddedObject)
//REMOVE    #endif

class ScDocShell;
struct TableLink_Impl;

class ScTableLink : public ::sfx2::SvBaseLink, public ScRefreshTimer
{
private:
    TableLink_Impl* pImpl;
    String          aFileName;
    String          aFilterName;
    String          aOptions;
    BOOL            bInCreate;
    BOOL            bInEdit;
    BOOL            bAddUndo;
    BOOL            bDoPaint;

public:
    TYPEINFO();
    ScTableLink( ScDocShell* pDocSh, const String& rFile,
                    const String& rFilter, const String& rOpt, ULONG nRefresh );
    ScTableLink( SfxObjectShell* pShell, const String& rFile,
                    const String& rFilter, const String& rOpt, ULONG nRefresh );
    virtual ~ScTableLink();
    virtual void Closed();
    virtual void DataChanged( const String& rMimeType,
                              const ::com::sun::star::uno::Any & rValue );

    virtual void    Edit( Window*, const Link& rEndEditHdl );

    BOOL    Refresh(const String& rNewFile, const String& rNewFilter,
                    const String* pNewOptions /* = NULL */, ULONG nNewRefresh );
    void    SetInCreate(BOOL bSet)      { bInCreate = bSet; }
    void    SetAddUndo(BOOL bSet)       { bAddUndo = bSet; }
    void    SetPaint(BOOL bSet)         { bDoPaint = bSet; }

    const String& GetFileName() const   { return aFileName; }
    const String& GetFilterName() const { return aFilterName; }
    const String& GetOptions() const    { return aOptions; }

    BOOL    IsUsed() const;

    DECL_LINK( RefreshHdl, ScTableLink* );
    DECL_LINK( EndEditHdl, ::sfx2::SvBaseLink* );
};

class ScDocument;
class SfxMedium;

class SC_DLLPUBLIC ScDocumentLoader
{
private:
    ScDocShell*         pDocShell;
    SfxObjectShellRef   aRef;
    SfxMedium*          pMedium;

public:
                        ScDocumentLoader( const String& rFileName,
                                            String& rFilterName, String& rOptions,
                                            UINT32 nRekCnt = 0, BOOL bWithInteraction = FALSE );
                        ~ScDocumentLoader();
    ScDocument*         GetDocument();
    ScDocShell*         GetDocShell()       { return pDocShell; }
    BOOL                IsError() const;
    String              GetTitle() const;

    void                ReleaseDocRef();    // without calling DoClose

    static String       GetOptions( SfxMedium& rMedium );

    /** Returns the filter name and options from a file name.
        @param bWithContent
            true = Tries to detect the filter by looking at the file contents.
            false = Detects filter by file name extension only (should be used in filter code only).
        @return TRUE if a filter could be found, FALSE otherwise. */
    static BOOL         GetFilterName( const String& rFileName,
                                        String& rFilter, String& rOptions,
                                        BOOL bWithContent );

    static void         RemoveAppPrefix( String& rFilterName );
};

#endif
