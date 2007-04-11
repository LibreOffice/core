/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: doctempl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:19:45 $
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
#ifndef _SFXDOCTEMPL_HXX
#define _SFXDOCTEMPL_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _REF_HXX //autogen
#include <tools/ref.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

// CLASS -----------------------------------------------------------------

class SfxObjectShell;

#ifndef SFX_DECL_OBJECTSHELL_DEFINED
#define SFX_DECL_OBJECTSHELL_DEFINED
SV_DECL_REF(SfxObjectShell)
#endif

class SfxDocTemplate_Impl;

#ifndef SFX_DECL_DOCTEMPLATES_DEFINED
#define SFX_DECL_DOCTEMPLATES_DEFINED
SV_DECL_REF(SfxDocTemplate_Impl)
#endif

// class SfxDocumentTemplates --------------------------------------------

class SFX2_DLLPUBLIC SfxDocumentTemplates
{
private:
    SfxDocTemplate_ImplRef  pImp;

    SAL_DLLPRIVATE BOOL CopyOrMove( USHORT nTargetRegion, USHORT nTargetIdx,
                                    USHORT nSourceRegion, USHORT nSourceIdx, BOOL bMove );
public:
                        SfxDocumentTemplates();
                        SfxDocumentTemplates(const SfxDocumentTemplates &);
                        ~SfxDocumentTemplates();

    BOOL                IsConstructed() { return pImp != NULL; }
    void                Construct();

    static BOOL         SaveDir( /*SfxTemplateDir &rEntry */ ) ;
    const SfxDocumentTemplates &operator=(const SfxDocumentTemplates &);

    BOOL                Rescan( );      // Aktualisieren
    void                ReInitFromComponent();

    BOOL                IsRegionLoaded( USHORT nIdx ) const;
    USHORT              GetRegionCount() const;
    const String&       GetRegionName(USHORT nIdx) const;                   //dv!
    String              GetFullRegionName(USHORT nIdx) const;
    USHORT              GetRegionNo( const String &rRegionName ) const;

    USHORT              GetCount(USHORT nRegion) const;
    USHORT              GetCount( const String &rName) const;
    const String&       GetName(USHORT nRegion, USHORT nIdx) const;         //dv!
    String              GetFileName(USHORT nRegion, USHORT nIdx) const;
    String              GetPath(USHORT nRegion, USHORT nIdx) const;

    String              GetDefaultTemplatePath(const String &rLongName);

    // Pfad zur Vorlage geben lassen; logischer Name muss angegeben
    // werden, damit beim Ueberschreiben einer Vorlage der
    // richtige Dateiname gefunden werden kann
    String              GetTemplatePath(USHORT nRegion, const String &rLongName) const;

    // Allows to retrieve the target template URL from the UCB
    ::rtl::OUString     GetTemplateTargetURLFromComponent( const ::rtl::OUString& aGroupName,
                                                         const ::rtl::OUString& aTitle );

    // Speichern als Vorlage hat geklappt -> Aktualisieren
    void            NewTemplate(USHORT nRegion,
                                const String &rLongName,
                                const String &rFileName);

    BOOL            Copy(USHORT nTargetRegion,
                         USHORT nTargetIdx,
                         USHORT nSourceRegion,
                         USHORT nSourceIdx);
    BOOL            Move(USHORT nTargetRegion,
                         USHORT nTargetIdx,
                         USHORT nSourceRegion,
                         USHORT nSourceIdx);
    BOOL            Delete(USHORT nRegion, USHORT nIdx);
    BOOL            InsertDir(const String &rText, USHORT nRegion);
    BOOL            SetName(const String &rName, USHORT nRegion, USHORT nIdx);

    BOOL            CopyTo(USHORT nRegion, USHORT nIdx, const String &rName) const;
    BOOL            CopyFrom(USHORT nRegion, USHORT nIdx, String &rName);

    SfxObjectShellRef CreateObjectShell(USHORT nRegion, USHORT nIdx);
    BOOL            DeleteObjectShell(USHORT, USHORT);

    BOOL            GetFull( const String& rRegion, const String& rName, String& rPath );
    BOOL            GetLogicNames( const String& rPath, String& rRegion, String& rName ) const;

    /** updates the configuration where the document templates structure is stored.

        <p>The info about the document templates (which files, which groups etc.) is stored in the
        configuration. This means that just by copying files into OOo's template directories, this
        change is not reflected in the SfxDocumentTemplates - 'cause the configuration is not synchronous with
        the file system. This can be enforced with this method.</p>

    @param _bSmart
        The update of the configuration is rather expensive - nothing you want to do regulary if you don't really
        need it. So you have the possibility to do a smart update - it first checks if the update if necessary.
        In case the update is needed, the additional check made it somewhat more expensive. In case it's not
        necessary (which should be the usual case), the check alone is (much) less expensive than the real update.
        <br/>
        So set <arg>_bSmart</arg> to <TRUE/> to do a check for necessity first.
    */
    void            Update( sal_Bool _bSmart = sal_True );

    // allows to detect whether it is allowed to delete ( at least partially )
    // a group or a template, or to edit a template
    sal_Bool        HasUserContents( sal_uInt16 nRegion, sal_uInt16 nIdx ) const;
};

#endif // #ifndef _SFXDOCTEMPL_HXX


