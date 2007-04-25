/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawdoc.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 08:53:18 $
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
#ifndef _DRAWDOC_HXX
#define _DRAWDOC_HXX

#ifndef _FM_FMMODEL_HXX
#include <svx/fmmodel.hxx>
#endif

class SwDoc;
class SwDocShell;

//==================================================================

class SwDrawDocument : public FmFormModel
{
    SwDoc* pDoc;
public:
    SwDrawDocument( SwDoc* pDoc );
    ~SwDrawDocument();

    const SwDoc& GetDoc() const { return *pDoc; }
          SwDoc& GetDoc()       { return *pDoc; }

    virtual SdrPage* AllocPage(FASTBOOL bMasterPage);

    // fuers "load on demand" von Grafiken im DrawingLayer
    virtual SvStream* GetDocumentStream( SdrDocumentStreamInfo& rInfo ) const;

    // fuers Speicher von Rechtecken als Control-Ersatz fuker Versionen < 5.0
    virtual SdrLayerID GetControlExportLayerId( const SdrObject & ) const;

protected:
    // --> OD 2006-03-01 #b6382898#
    // overload of <SdrModel::createUnoModel()> is needed to provide corresponding uno model.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoModel();
    // <--
};


#endif
