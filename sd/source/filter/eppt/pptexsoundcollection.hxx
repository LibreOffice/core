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

#ifndef _SD_PPT_EXSOUNDCOLLECTION_HXX
#define _SD_PPT_EXSOUNDCOLLECTION_HXX

#include <boost/ptr_container/ptr_vector.hpp>

#ifdef DBG_ANIM_LOG
#include <stdio.h>
#endif
#include <rtl/ustring.hxx>
#include <tools/stream.hxx>

namespace ppt
{

class ExSoundEntry
{
        sal_uInt32              nFileSize;
        rtl::OUString           aSoundURL;

        rtl::OUString           ImplGetName() const;
        rtl::OUString           ImplGetExtension() const;

    public :

        sal_Bool                IsSameURL(const rtl::OUString& rURL) const;
        sal_uInt32              GetFileSize( ) const { return nFileSize; };

                                ExSoundEntry(const rtl::OUString& rSoundURL);

        /// @return size of a complete SoundContainer.
        sal_uInt32              GetSize( sal_uInt32 nId ) const;
        void                    Write( SvStream& rSt, sal_uInt32 nId ) const;
};

class ExSoundCollection
{
    public:

        sal_uInt32              GetId(const rtl::OUString&);

        /// @return size of a complete SoundCollectionContainer.
        sal_uInt32              GetSize() const;
        void                    Write( SvStream& rSt ) const;

private:

    boost::ptr_vector<ExSoundEntry> maEntries;
};

} // namespace ppt

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
