/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SFX2_DOCTEMPL_HXX
#define INCLUDED_SFX2_DOCTEMPL_HXX

#include <sal/config.h>

#include <string_view>

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <sfx2/dllapi.h>
#include <tools/ref.hxx>

// CLASS -----------------------------------------------------------------

class SfxDocTemplate_Impl;

// class SfxDocumentTemplates --------------------------------------------

class SFX2_DLLPUBLIC SfxDocumentTemplates
{
private:
    tools::SvRef<SfxDocTemplate_Impl> pImp;

    SAL_DLLPRIVATE bool CopyOrMove( sal_uInt16 nTargetRegion, sal_uInt16 nTargetIdx,
                                    sal_uInt16 nSourceRegion, sal_uInt16 nSourceIdx, bool bMove );
public:
                        SfxDocumentTemplates();
                        SfxDocumentTemplates(const SfxDocumentTemplates &);
                        ~SfxDocumentTemplates();

    const SfxDocumentTemplates &operator=(const SfxDocumentTemplates &);

    void                ReInitFromComponent();

    sal_uInt16          GetRegionCount() const;
    OUString            GetRegionName(sal_uInt16 nIdx) const;                   //dv!
    OUString            GetFullRegionName(sal_uInt16 nIdx) const;

    sal_uInt16          GetCount(sal_uInt16 nRegion) const;
    OUString            GetName(sal_uInt16 nRegion, sal_uInt16 nIdx) const;         //dv!
    OUString            GetPath(sal_uInt16 nRegion, sal_uInt16 nIdx) const;

    // Allows to retrieve the target template URL from the UCB
    OUString     GetTemplateTargetURLFromComponent(std::u16string_view aGroupName,
                                                         std::u16string_view aTitle );

    // Convert a resource string - a template name - to its localised pair
    // if it exists in sfx2/inc/strings.hxx
    static OUString ConvertResourceString(const OUString& rString);

    bool            Copy(sal_uInt16 nTargetRegion,
                         sal_uInt16 nTargetIdx,
                         sal_uInt16 nSourceRegion,
                         sal_uInt16 nSourceIdx);
    bool            Move(sal_uInt16 nTargetRegion,
                         sal_uInt16 nTargetIdx,
                         sal_uInt16 nSourceRegion,
                         sal_uInt16 nSourceIdx);
    bool            Delete(sal_uInt16 nRegion, sal_uInt16 nIdx);
    bool            InsertDir(const OUString &rText, sal_uInt16 nRegion);

    bool InsertTemplate (sal_uInt16 nSourceRegion, sal_uInt16 nIdx, const OUString &rName, const OUString &rPath);

    /** Change the name of an entry or a directory

        \param rName
            The new name to set
        \param nRegion
            The id of the region to rename or containing the template to rename
        \param nIdx
            The id of the template to rename or USHRT_MAX to rename the region.

        \return
            sal_True if the action could be performed, sal_False otherwise

    */
    bool            SetName(const OUString &rName, sal_uInt16 nRegion, sal_uInt16 nIdx);

    bool            CopyTo(sal_uInt16 nRegion, sal_uInt16 nIdx, const OUString &rName) const;
    bool            CopyFrom(sal_uInt16 nRegion, sal_uInt16 nIdx, OUString &rName);

    bool            GetFull(
        std::u16string_view rRegion, std::u16string_view rName, OUString& rPath );
    bool            GetLogicNames( const OUString& rPath, OUString& rRegion, OUString& rName ) const;

    /** updates the configuration where the document templates structure is stored.

        <p>The info about the document templates (which files, which groups etc.) is stored in the
        configuration. This means that just by copying files into OOo's template directories, this
        change is not reflected in the SfxDocumentTemplates - 'cause the configuration is not synchronous with
        the file system. This can be enforced with this method.</p>
    */
    void            Update();
};

#endif // INCLUDED_SFX2_DOCTEMPL_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
