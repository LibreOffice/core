/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: databaselocationinput.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 17:11:01 $
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

#ifndef SVX_DATABASELOCATIONINPUT_HXX
#define SVX_DATABASELOCATIONINPUT_HXX

#include "svx/svxdllapi.h"

/** === begin UNO includes === **/
/** === end UNO includes === **/

class PushButton;
class String;
namespace svt { class OFileURLControl; }
namespace comphelper { class ComponentContext; }

#include <memory>

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= DatabaseLocationInputController
    //====================================================================
    class DatabaseLocationInputController_Impl;
    /** helper class to control controls needed to input a database location

        If you allow, in your dialog, to save a database document, then you usually
        have a OFileURLControl for inputting the actual location, and a push button
        to browse for a location.

        This helper class controls such two UI elements.
    */
    class SVX_DLLPUBLIC DatabaseLocationInputController
    {
    public:
        DatabaseLocationInputController(
            const ::comphelper::ComponentContext&   _rContext,
            ::svt::OFileURLControl&                 _rLocationInput,
            PushButton&                             _rBrowseButton
        );
        ~DatabaseLocationInputController();

        /** sets the given URL at the input control, after translating it into a system path
        */
        void    setURL( const String& _rURL );

        /** returns the current database location, in form of an URL (not a system path)
        */
        String  getURL() const;

        /** prepares committing the database location entered in the input field

            Effectively, this method checks whether the file in the location already
            exists, and if so, it asks the user whether to overwrite it.

            If the method is called multiple times, this check only happens when the location
            changed since the last call.
        */
        bool    prepareCommit();

    private:
        ::std::auto_ptr< DatabaseLocationInputController_Impl >
                m_pImpl;
    };

//........................................................................
} // namespace svx
//........................................................................

#endif // SVX_DATABASELOCATIONINPUT_HXX
