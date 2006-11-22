/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docinsert.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-22 10:53:30 $
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

