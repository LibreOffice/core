#ifndef _REPORT_SECTIONVIEW_HXX
#define _REPORT_SECTIONVIEW_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SectionView.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:30 $
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



#ifndef _SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif
namespace rptui
{
class OReportWindow;
class OReportSection;

//============================================================================
// OSectionView
//============================================================================

class OSectionView : public SdrView
{
private:
    OReportWindow*      m_pReportWindow;
    OReportSection*     m_pSectionWindow;

    void ObjectRemovedInAliveMode( const SdrObject* pObject );
    OSectionView(const OSectionView&);
    void operator =(const OSectionView&);
public:
    TYPEINFO();

    OSectionView( SdrModel* pModel, OReportSection* _pSectionWindow, OReportWindow* pEditor );
    virtual ~OSectionView();

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual void MarkListHasChanged();
    virtual void MakeVisible( const Rectangle& rRect, Window& rWin );

    inline OReportSection*  getSectionWindow() const { return m_pSectionWindow; }
};
}
#endif //_REPORT_SECTIONVIEW_HXX
