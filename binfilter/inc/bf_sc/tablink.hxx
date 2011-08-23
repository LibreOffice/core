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

#ifndef SC_TABLINK_HXX
#define SC_TABLINK_HXX

#ifndef SC_REFRESHTIMER_HXX
#include "refreshtimer.hxx"
#endif

#ifndef _EMBOBJ_HXX 
#include <bf_so3/embobj.hxx>
#endif

#ifndef _LNKBASE_HXX //autogen
#include <bf_so3/lnkbase.hxx>
#endif
namespace binfilter {

#ifndef SO2_DECL_SVEMBEDDEDOBJECT_DEFINED
#define SO2_DECL_SVEMBEDDEDOBJECT_DEFINED
SO2_DECL_REF(SvEmbeddedObject)
#endif

class ScDocShell;
class SfxObjectShell;

class ScTableLink : public ::binfilter::SvBaseLink, public ScRefreshTimer
{
private:
    ScDocShell*	pDocShell;		// Container
    String		aFileName;
    String		aFilterName;
    String		aOptions;
    BOOL		bInCreate;
    BOOL		bInEdit;
    BOOL		bAddUndo;
    BOOL		bDoPaint;

public:
    TYPEINFO();
    ScTableLink( ScDocShell* pDocSh, const String& rFile,
                    const String& rFilter, const String& rOpt, ULONG nRefresh );
    ScTableLink( SfxObjectShell* pShell, const String& rFile,
                    const String& rFilter, const String& rOpt, ULONG nRefresh );
    virtual ~ScTableLink();
    virtual void DataChanged( const String& rMimeType,
                              const ::com::sun::star::uno::Any & rValue );


    BOOL	Refresh(const String& rNewFile, const String& rNewFilter,
                    const String* pNewOptions /* = NULL */, ULONG nNewRefresh );
    void	SetInCreate(BOOL bSet)		{ bInCreate = bSet; }
    void	SetAddUndo(BOOL bSet)		{ bAddUndo = bSet; }
    void	SetPaint(BOOL bSet)			{ bDoPaint = bSet; }

    const String& GetFileName() const	{ return aFileName; }
    const String& GetFilterName() const	{ return aFilterName; }
    const String& GetOptions() const	{ return aOptions; }


    DECL_LINK( RefreshHdl, ScTableLink* );

};


class ScDocument;
class SfxMedium;

class ScDocumentLoader
{
private:
    ScDocShell*			pDocShell;
    SvEmbeddedObjectRef aRef;
    SfxMedium*			pMedium;

public:
                        ScDocumentLoader( const String& rFileName,
                                            String& rFilterName, String& rOptions,
                                            UINT32 nRekCnt = 0, BOOL bWithInteraction = FALSE );
                        ~ScDocumentLoader();
    ScDocument*			GetDocument();
    ScDocShell*			GetDocShell()		{ return pDocShell; }
    BOOL				IsError() const;

    static String		GetOptions( SfxMedium& rMedium );
    static void			GetFilterName( const String& rFileName,
                                        String& rFilter, String& rOptions,
                                        BOOL bWithContent = FALSE );
    static void			RemoveAppPrefix( String& rFilterName );
};

} //namespace binfilter
#endif

