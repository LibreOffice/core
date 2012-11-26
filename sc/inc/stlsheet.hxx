/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_STLSHEET_HXX
#define SC_STLSHEET_HXX

#include <svl/style.hxx>

//------------------------------------------------------------------------

#define SCSTYLEBIT_STANDARD     0x0001

//------------------------------------------------------------------------

class ScStyleSheetPool;

class ScStyleSheet : public SfxStyleSheet
{
friend class ScStyleSheetPool;

public:

    enum    Usage
    {
        UNKNOWN,
        USED,
        NOTUSED
    };

private:
    mutable ScStyleSheet::Usage eUsage;

public:
                        ScStyleSheet( const ScStyleSheet& rStyle );

    virtual sal_Bool        SetParent        ( const String& rParentName );
    virtual SfxItemSet& GetItemSet       ();
    virtual sal_Bool        IsUsed           () const;
    virtual sal_Bool        HasFollowSupport () const;
    virtual sal_Bool        HasParentSupport () const;

    virtual const String& GetName() const;
    virtual const String& GetParent() const;
    virtual const String& GetFollow() const;

    virtual sal_Bool SetName( const String& );

            void                SetUsage( ScStyleSheet::Usage eUse ) const
                                    { eUsage = eUse; }
            ScStyleSheet::Usage GetUsage() const
                                    { return eUsage; }

protected:
    virtual             ~ScStyleSheet();

                ScStyleSheet( const String&     rName,
                              ScStyleSheetPool& rPool,
                              SfxStyleFamily    eFamily,
                              sal_uInt16            nMask );

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

#endif     // SC_STLSHEET_HXX

