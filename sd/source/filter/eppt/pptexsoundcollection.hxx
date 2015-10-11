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

#ifndef INCLUDED_SD_SOURCE_FILTER_EPPT_PPTEXSOUNDCOLLECTION_HXX
#define INCLUDED_SD_SOURCE_FILTER_EPPT_PPTEXSOUNDCOLLECTION_HXX

#include <boost/ptr_container/ptr_vector.hpp>

#ifdef DBG_ANIM_LOG
#include <stdio.h>
#endif
#include <rtl/ustring.hxx>

class SvStream;

namespace ppt
{

class ExSoundEntry
{
        sal_uInt32              nFileSize;
        OUString           aSoundURL;

        OUString           ImplGetName() const;
        OUString           ImplGetExtension() const;

    public:

        bool                IsSameURL(const OUString& rURL) const;
        sal_uInt32              GetFileSize( ) const { return nFileSize; };

                                ExSoundEntry(const OUString& rSoundURL);

        /// @return size of a complete SoundContainer.
        sal_uInt32              GetSize( sal_uInt32 nId ) const;
        void                    Write( SvStream& rSt, sal_uInt32 nId ) const;
};

class ExSoundCollection
{
    public:

        sal_uInt32              GetId(const OUString&);

        /// @return size of a complete SoundCollectionContainer.
        sal_uInt32              GetSize() const;
        void                    Write( SvStream& rSt ) const;

private:

    boost::ptr_vector<ExSoundEntry> maEntries;
};

} // namespace ppt

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
