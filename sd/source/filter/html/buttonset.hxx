/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: buttonset.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:38:53 $
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

#ifndef _SD_HTMLEX_BUTTONSET_HXX
#define _SD_HTMLEX_BUTTONSET_HXX

#include <rtl/ustring.hxx>
#include <boost/scoped_ptr.hpp>
#include <vector>

class Image;
class ButtonSetImpl;

class ButtonSet
{
public:
    ButtonSet();
    ~ButtonSet();

    int getCount() const;

    bool getPreview( int nSet, const std::vector< rtl::OUString >& rButtons, Image& rImage );
    bool exportButton( int nSet, const rtl::OUString& rPath, const rtl::OUString& rName );

private:
    ButtonSetImpl* mpImpl;
};

#endif // _SD_HTMLEX_BUTTONSET_HXX
