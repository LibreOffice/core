/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
            accessNull = 0,
            accessWritable = 0,
            accessFinal = 1,
            accessReadonly = 2,
            accessReadonlyAndFinal = 3
        };
        inline Access makeAccess(bool readonly, bool final)
        { return Access( (readonly ? accessReadonly : accessNull) | (final ? accessFinal : accessNull) ); }
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
            : state_(node::isMerged)
            , bReadonly(false)
            , bFinalized(false)
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
