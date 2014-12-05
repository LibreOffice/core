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

#ifndef INCLUDED_OOX_PPT_PRESENTATIONFRAGMENTHANDLER_HXX
#define INCLUDED_OOX_PPT_PRESENTATIONFRAGMENTHANDLER_HXX

#include <oox/ppt/slidepersist.hxx>
#include <oox/ppt/customshowlistcontext.hxx>

namespace oox { namespace ppt {

class PresentationFragmentHandler : public ::oox::core::FragmentHandler2
{
public:
    PresentationFragmentHandler( ::oox::core::XmlFilterBase& rFilter, const OUString& rFragmentPath ) throw();
    virtual ~PresentationFragmentHandler() throw();
    virtual void finalizeImport() SAL_OVERRIDE;
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) SAL_OVERRIDE;

protected:
    bool importSlide( const ::oox::core::FragmentHandlerRef& rxSlideFragmentHandler,
                        const oox::ppt::SlidePersistPtr pPersist );

private:

    void importSlide(sal_uInt32 nSlide, bool bFirstSlide, bool bImportNotes);

    std::vector< OUString > maSlideMasterVector;
    std::vector< OUString > maSlidesVector;
    std::vector< OUString > maNotesMasterVector;
    ::oox::drawingml::TextListStylePtr mpTextListStyle;

    ::com::sun::star::awt::Size maSlideSize;
    ::com::sun::star::awt::Size maNotesSize;

    std::vector< CustomShow >   maCustomShowList;

    CommentAuthorList           maAuthorList;
    bool                        mbCommentAuthorsRead; // read commentAuthors.xml only once
};

} }

#endif // INCLUDED_OOX_PPT_PRESENTATIONFRAGMENTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
