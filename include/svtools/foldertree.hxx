/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVTOOLS_FOLDERTREE_HXX
#define INCLUDED_SVTOOLS_FOLDERTREE_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>

#include <officecfg/Office/Common.hxx>

#include <svtools/svtdllapi.h>
#include <svtools/svtools.hrc>
#include <svtools/svtresid.hxx>
#include <svtools/treelistentry.hxx>

#include <tools/urlobj.hxx>

#include <ucbhelper/commandenvironment.hxx>

#include <vcl/image.hxx>
#include <vcl/lstbox.hxx>

using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::svt;

class SVT_DLLPUBLIC FolderTree : public SvTreeListBox
{
private:
    Reference< XCommandEnvironment > m_xEnv;
    ::osl::Mutex m_aMutex;
    Sequence< OUString > m_aBlackList;
    Image m_aFolderImage;
    Image m_aFolderExpandedImage;

    OUString m_sLastUpdatedDir;

public:
    FolderTree( vcl::Window* pParent, WinBits nBits );

    virtual void RequestingChildren( SvTreeListEntry* pEntry ) override;

    void FillTreeEntry( SvTreeListEntry* pEntry );
    void FillTreeEntry( const OUString & rUrl, const ::std::vector< std::pair< OUString, OUString > >& rFolders );
    void SetTreePath( OUString const & sUrl );
    void SetBlackList( const ::com::sun::star::uno::Sequence< OUString >& rBlackList );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
