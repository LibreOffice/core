/*************************************************************************
 *
 *  $RCSfile: attributes.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:32:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_CONFIGURATION_ATTRIBUTES_HXX_
#define CONFIGMGR_CONFIGURATION_ATTRIBUTES_HXX_
namespace configmgr
{
    namespace node
    {
        enum State
        {
            isDefault,          isToDefault     = isDefault,
            isMerged,           isModification  = isMerged,
            isReplaced,         isReplacement   = isReplaced,
            isAdded,            isAddition      = isAdded
        };
        enum Access
        {
            accessWritable,
            accessFinal,
            accessReadonly,
            accessReadonlyAndFinal
        };
        inline Access makeAccess(bool readonly, bool final)
        { return Access( (readonly ? accessReadonly : 0u) | (final ? accessFinal : 0u) ); }
        inline bool isAccessReadonly(Access access)
        { return (access & accessReadonly) != 0; }
        inline bool isAccessFinal(Access access)
        { return (access & accessFinal) != 0; }

        inline bool existsInDefault(State eState)   { return eState <= isReplaced;}
        inline bool isReplacedForUser(State eState) { return eState >= isReplaced;}

        /// holds attributes a node in the schema
        struct Attributes
        {
            Attributes()
            : bReadonly(false)
            , bFinalized(false)
            , state_(node::isMerged)
            , bNullable(true)
            , bLocalized(false)
            , bMandatory(false)
            , bRemovable(false)
            {}

            State state() const         { return State(0x03 & state_); }
            void setState(State _state) { this->state_ = _state; }

            bool isWritable() const              { return!bReadonly; }
            bool isReadonly() const              { return bReadonly; }
            bool isFinalized() const             { return bFinalized; }

            void markReadonly() { bReadonly = true; }

            Access getAccess() const
            { return makeAccess(bReadonly,bFinalized); }

            void setAccess(bool _bReadonly, bool _bFinalized)
            { bReadonly = _bReadonly; bFinalized = _bFinalized; }

            void setAccess(Access _aAccessLevel)
            { setAccess( isAccessReadonly(_aAccessLevel), isAccessFinal(_aAccessLevel) );  }

            bool isNullable() const              { return bNullable; }
            void setNullable (bool _bNullable)   {bNullable = _bNullable; }

            bool isLocalized() const             { return bLocalized; }
            void setLocalized (bool _bLocalized) {bLocalized = _bLocalized; }

            bool isMandatory() const             { return bMandatory; }
            bool isRemovable() const             { return bRemovable; }

            void markMandatory() { bMandatory = true; }
            void markRemovable() { bRemovable = true; }

            void setRemovability(bool _bRemovable, bool _bMandatory)
            { bRemovable = _bRemovable; bMandatory = _bMandatory; }

            bool isDefault()            const { return this->state() == node::isDefault;}
            bool existsInDefault()      const { return node::existsInDefault(this->state());}
            bool isReplacedForUser()    const { return node::isReplacedForUser(this->state());}

            void markAsDefault(bool _bDefault = true)
            {
                if (_bDefault)
                    this->state_ = node::isDefault;
                else if (this->isDefault())
                    this->state_ = node::isMerged;
            }

        private:
            State state_        : 2;    // merged/replaced/default state

            bool bReadonly      : 1;    // write-protected, if true
            bool bFinalized     : 1;    // can not be overridden - write protected when merged upwards

            bool bNullable      : 1;    // values only: can be NULL
            bool bLocalized     : 1;    // values only: value may depend on locale

            bool bMandatory     : 1;    // cannot be removed/replaced in subsequent layers
            bool bRemovable     : 1;    // can be removed
        };

    }
}

#endif
