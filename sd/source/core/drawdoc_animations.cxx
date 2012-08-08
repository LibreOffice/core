/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "drawdoc.hxx"
#include "cusshow.hxx"
#include "customshowlist.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;

/** replaces a slide from all custom shows with a new one or removes a slide from
    all custom shows if pNewPage is 0.
*/
void SdDrawDocument::ReplacePageInCustomShows( const SdPage* pOldPage, const SdPage* pNewPage )
{
    if ( mpCustomShowList )
    {
        for (sal_uLong i = 0; i < mpCustomShowList->size(); i++)
        {
            SdCustomShow* pCustomShow = (*mpCustomShowList)[i];
            pCustomShow->ReplacePage(pOldPage, pNewPage);
        }
    }
}

extern Reference< XPresentation2 > CreatePresentation( const SdDrawDocument& rDocument );

const Reference< XPresentation2 >& SdDrawDocument::getPresentation() const
{
    if( !mxPresentation.is() )
    {
        const_cast< SdDrawDocument* >( this )->mxPresentation = CreatePresentation(*this);
    }
    return mxPresentation;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
