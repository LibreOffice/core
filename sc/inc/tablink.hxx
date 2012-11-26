/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_TABLINK_HXX
#define SC_TABLINK_HXX

#include "scdllapi.h"
#include "refreshtimer.hxx"
#include <sfx2/lnkbase.hxx>

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
    sal_Bool            bInCreate;
    sal_Bool            bInEdit;
    sal_Bool            bAddUndo;
    sal_Bool            bDoPaint;

public:
    ScTableLink( ScDocShell* pDocSh, const String& rFile,
                    const String& rFilter, const String& rOpt, sal_uLong nRefresh );
    ScTableLink( SfxObjectShell* pShell, const String& rFile,
                    const String& rFilter, const String& rOpt, sal_uLong nRefresh );
    virtual ~ScTableLink();
    virtual void Closed();
    virtual void DataChanged( const String& rMimeType,
                              const ::com::sun::star::uno::Any & rValue );

    virtual void    Edit( Window*, const Link& rEndEditHdl );

    sal_Bool    Refresh(const String& rNewFile, const String& rNewFilter,
                    const String* pNewOptions /* = NULL */, sal_uLong nNewRefresh );
    void    SetInCreate(sal_Bool bSet)      { bInCreate = bSet; }
    void    SetAddUndo(sal_Bool bSet)       { bAddUndo = bSet; }
    void    SetPaint(sal_Bool bSet)         { bDoPaint = bSet; }

    const String& GetFileName() const   { return aFileName; }
    const String& GetFilterName() const { return aFilterName; }
    const String& GetOptions() const    { return aOptions; }

    sal_Bool    IsUsed() const;

    DECL_LINK( RefreshHdl, ScTableLink* );
    DECL_LINK( TableEndEditHdl, ::sfx2::SvBaseLink* );
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
                                            sal_uInt32 nRekCnt = 0, sal_Bool bWithInteraction = sal_False );
                        ~ScDocumentLoader();
    ScDocument*         GetDocument();
    ScDocShell*         GetDocShell()       { return pDocShell; }
    sal_Bool                IsError() const;
    String              GetTitle() const;

    void                ReleaseDocRef();    // without calling DoClose

    static String       GetOptions( SfxMedium& rMedium );

    /** Returns the filter name and options from a file name.
        @param bWithContent
            true = Tries to detect the filter by looking at the file contents.
            false = Detects filter by file name extension only (should be used in filter code only).
        @return sal_True if a filter could be found, sal_False otherwise. */
    static sal_Bool         GetFilterName( const String& rFileName,
                                        String& rFilter, String& rOptions,
                                        sal_Bool bWithContent, sal_Bool bWithInteraction );

    static void         RemoveAppPrefix( String& rFilterName );
};

#endif
