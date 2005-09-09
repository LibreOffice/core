/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FactoryIds.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:51:22 $
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

#ifndef SD_FACTORY_IDS_HXX
#define SD_FACTORY_IDS_HXX

namespace sd {

/** This are ids used by SfxTopFrame::Create() or CreateViewFrame() to
    select a factory to create an instance of an SfxViewShell super
    class.  This allows the caller to create a view shell directly
    with a certain view instead of first create a default view and
    then switch to the desired view.
*/
enum ViewShellFactoryIds
{
    IMPRESS_FACTORY_ID = 1,
    DRAW_FACTORY_ID = 1,
    SLIDE_SORTER_FACTORY_ID = 2,
    OUTLINE_FACTORY_ID = 3,
    PRESENTATION_FACTORY_ID = 4,
    PREVIEW_FACTORY_ID = 5
};

} // end of namespace sd

#endif

