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



#ifndef _SDR_MASTERPAGEDESCRIPTOR_HXX
#define _SDR_MASTERPAGEDESCRIPTOR_HXX

#include <svx/svdsob.hxx>
#include <svl/lstner.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrPage;
namespace sdr { namespace contact { class ViewContact; }}
class SdrPageProperties;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    class MasterPageDescriptor : public SfxListener
    {
    private:
        SdrPage&                                        maOwnerPage;
        SdrPage&                                        maUsedPage;
        SetOfByte                                       maVisibleLayers;

        // ViewContact part
        sdr::contact::ViewContact*                      mpViewContact;

        // assignment operator defined, but not implemented
        void operator=(const MasterPageDescriptor& rCandidate);

    protected:
        virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

    public:
        MasterPageDescriptor(SdrPage& aOwnerPage, SdrPage& aUsedPage);
        virtual ~MasterPageDescriptor();

        // ViewContact part
        sdr::contact::ViewContact& GetViewContact() const;

        // this method is called form the destructor of the referenced page.
        // do all necessary action to forget the page. It is not necessary to call
        // RemovePageUser(), that is done form the destructor.
        virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

        // member access to UsedPage
        SdrPage& GetUsedPage() const { return maUsedPage; }

        // member access to OwnerPage
        SdrPage& GetOwnerPage() const { return maOwnerPage; }

        // member access to VisibleLayers
        const SetOfByte& GetVisibleLayers() const { return maVisibleLayers; }
        void SetVisibleLayers(const SetOfByte& rNew);

        // operators
        bool operator==(const MasterPageDescriptor& rCandidate) const;
        bool operator!=(const MasterPageDescriptor& rCandidate) const;

        const SdrPageProperties* getCorrectSdrPageProperties() const;
    };
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_MASTERPAGEDESCRIPTOR_HXX
// eof
