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



#ifndef _SFX_DOCINSERT_HXX
#define _SFX_DOCINSERT_HXX

#include <tools/errcode.hxx>
#include <tools/link.hxx>
#include <tools/string.hxx>

#include "sfx2/dllapi.h"

namespace sfx2 { class FileDialogHelper; }
class SfxMedium;
class SfxMediumList;
class SfxItemSet;
class SvStringsDtor;

// ============================================================================

namespace sfx2 {

// ============================================================================

// ============================================================================
// DocumentInserter
// ============================================================================

class SFX2_DLLPUBLIC DocumentInserter
{
private:
    String                  m_sDocFactory;
    String                  m_sFilter;
    Link                    m_aDialogClosedLink;

    bool                    m_bMultiSelectionEnabled;
    sal_Int64               m_nDlgFlags;
    ErrCode                 m_nError;

    sfx2::FileDialogHelper* m_pFileDlg;
    SfxItemSet*             m_pItemSet;
    SvStringsDtor*          m_pURLList;

    DECL_LINK(              DialogClosedHdl, sfx2::FileDialogHelper* );

public:
    DocumentInserter( sal_Int64 _nFlags, const String& _rFactory, bool _bEnableMultiSelection = false );
    ~DocumentInserter();

    void                    StartExecuteModal( const Link& _rDialogClosedLink );
    SfxMedium*              CreateMedium();
    SfxMediumList*          CreateMediumList();

    inline ErrCode          GetError() const { return m_nError; }
    inline String           GetFilter() const { return m_sFilter; }
};

// ============================================================================

} // namespace sfx2

// ============================================================================

#endif // _SFX_DOCINSERT_HXX

