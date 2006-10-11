/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IDocumentLayoutAccess.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-11 08:43:07 $
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

#ifndef IDOCUMENTLAYOUTACCESS_HXX_INCLUDED
#define IDOCUMENTLAYOUTACCESS_HXX_INCLUDED

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif

class SwRootFrm;
class SwFrmFmt;
class SfxItemSet;
class SwLayouter;
class SwFmtAnchor;

/** Provides access to the layout of a document.
*/
class IDocumentLayoutAccess
{
public:

    /** Returns the layout set at the document.
    */
    virtual const SwRootFrm* GetRootFrm() const = 0;
    virtual       SwRootFrm* GetRootFrm() = 0;

    /** !!!The old layout must be deleted!!!
    */
    virtual void SetRootFrm( SwRootFrm* pNew ) = 0;

    /**
    */
    virtual SwFrmFmt* MakeLayoutFmt( RndStdIds eRequest, const SfxItemSet* pSet ) = 0;

    /**
    */
    virtual SwLayouter* GetLayouter() = 0;
    virtual const SwLayouter* GetLayouter() const = 0;
    virtual void SetLayouter( SwLayouter* pNew ) = 0;

    /**
    */
    virtual void DelLayoutFmt( SwFrmFmt *pFmt ) = 0;

    /**
    */
    virtual SwFrmFmt* CopyLayoutFmt( const SwFrmFmt& rSrc, const SwFmtAnchor& rNewAnchor,
                                     bool bSetTxtFlyAtt, bool bMakeFrms ) = 0;

protected:

    virtual ~IDocumentLayoutAccess() {};
 };

 #endif // IDOCUMENTLAYOUTACCESS_HXX_INCLUDED
