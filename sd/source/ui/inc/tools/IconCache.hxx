/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IconCache.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:03:39 $
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

#ifndef SD_ICON_CACHE_HXX
#define SD_ICON_CACHE_HXX

#include "SdGlobalResourceContainer.hxx"
#include <vcl/image.hxx>

namespace sd {

/** This simple class stores frequently used icons so that the classes that
    use the icons do not have to store them in every one of their
    instances.

    Icons are adressed over their resource id and are loaded on demand.

    This cache acts like a singleton with a lifetime equal to that of the sd
    module.
*/
class IconCache
    : public SdGlobalResource
{
public:
    /** The lifetime of the returned reference is limited to that of the sd
        module.
    */
    static IconCache& Instance (void);

    /** Return the icon with the given resource id.
        @return
            The returned Image may be empty when there is no icon for the
            given id or an error occured.  Should not happen under normal
            circumstances.
    */
    Image GetIcon (USHORT nResourceId);

private:
    class Implementation;
    ::std::auto_ptr<Implementation> mpImpl;

    /** The constructor creates the one instance of the cache and registers
        it at the SdGlobalResourceContainer to limit is lifetime to that of
        the sd module.
    */
    IconCache (void);

    /** This destructor is called by SdGlobalResourceContainer.
    */
    virtual ~IconCache (void);
};

} // end of namespace sd

#endif
