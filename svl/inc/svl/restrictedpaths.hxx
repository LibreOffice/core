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



#ifndef SVTOOLS_RESTRICTEDPATHS_HXX
#define SVTOOLS_RESTRICTEDPATHS_HXX

#include <svl/urlfilter.hxx>
#include <svl/svldllapi.h>
#include <tools/string.hxx>

#include <vector>

namespace svt
{
    class SVL_DLLPUBLIC RestrictedPaths : public IUrlFilter
    {
    private:
        ::std::vector< String >     m_aUnrestrictedURLs;
        bool                        m_bFilterIsEnabled;

    public:
        RestrictedPaths();
        virtual ~RestrictedPaths();

        inline bool hasFilter() const { return !m_aUnrestrictedURLs.empty(); }
        inline const ::std::vector< String >& getFilter() const { return m_aUnrestrictedURLs; }

        inline void enableFilter( bool _bEnable ) { m_bFilterIsEnabled = _bEnable; }
        inline bool isFilterEnabled() const { return m_bFilterIsEnabled; }

    public:
        /** checks URL access permissions

            <p>with the "restriction" feature we have in the file dialog, it's possible that
            only certain URLs can be browsed. This method checks whether a given URL belongs
            to this set of permitted URLs.</p>

            <p>If no "access restriction" is effective, this method always returns <TRUE/>.</p>
        */
        virtual bool                isUrlAllowed( const String& _rURL ) const;

        /** checks URL access permissions

            <p>with the "restriction" feature we have in the file dialog, it's possible that
            only certain URLs can be browsed. This method checks whether a given URL belongs
            to this set of permitted URLs.</p>

            <p>Default behavior allows access to parent folder of a restricted folder (but not to its siblings).
            If allowParents is set to <FALSE/> parent folders will be treated as forbidden.

            <p>If no "access restriction" is effective, this method always returns <TRUE/>.</p>
        */
        bool                        isUrlAllowed( const String& _rURL, bool allowParents ) const;
    };

}   // namespace svt

#endif  // SVTOOLS_RESTRICTEDPATHS_HXX
