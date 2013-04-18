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

#ifndef SVTOOLS_RESTRICTEDPATHS_HXX
#define SVTOOLS_RESTRICTEDPATHS_HXX

#include <svl/urlfilter.hxx>
#include <svl/svldllapi.h>

#include <vector>

namespace svt
{
    class SVL_DLLPUBLIC RestrictedPaths : public IUrlFilter
    {
    private:
        ::std::vector< OUString >     m_aUnrestrictedURLs;
        bool                        m_bFilterIsEnabled;

    public:
        RestrictedPaths();
        virtual ~RestrictedPaths();

        inline bool hasFilter() const { return !m_aUnrestrictedURLs.empty(); }
        inline const ::std::vector< OUString >& getFilter() const { return m_aUnrestrictedURLs; }

        inline void enableFilter( bool _bEnable ) { m_bFilterIsEnabled = _bEnable; }
        inline bool isFilterEnabled() const { return m_bFilterIsEnabled; }

    public:
        /** checks URL access permissions

            <p>with the "restriction" feature we have in the file dialog, it's possible that
            only certain URLs can be browsed. This method checks whether a given URL belongs
            to this set of permitted URLs.</p>

            <p>If no "access restriction" is effective, this method always returns <TRUE/>.</p>
        */
        virtual bool                isUrlAllowed( const OUString& _rURL ) const;

        /** checks URL access permissions

            <p>with the "restriction" feature we have in the file dialog, it's possible that
            only certain URLs can be browsed. This method checks whether a given URL belongs
            to this set of permitted URLs.</p>

            <p>Default behavior allows access to parent folder of a restricted folder (but not to its siblings).
            If allowParents is set to <FALSE/> parent folders will be treated as forbidden.

            <p>If no "access restriction" is effective, this method always returns <TRUE/>.</p>
        */
        bool                        isUrlAllowed( const OUString& _rURL, bool allowParents ) const;
    };

}   // namespace svt

#endif  // SVTOOLS_RESTRICTEDPATHS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
