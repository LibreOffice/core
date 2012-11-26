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



#ifndef _MSGNODEI_HXX
#define _MSGNODEI_HXX


#include <svl/eitem.hxx>
#include <tools/string.hxx>
#include <svl/poolitem.hxx>
#include <tools/list.hxx>
struct SfxMsgAttachFile {
    String  aFile;
    String  aName;

    int                 operator==( const SfxMsgAttachFile& rRec ) const
                        {
                            if( aName == rRec.aName && aFile == rRec.aFile )
                                return sal_True;
                            return sal_False;
                        }

                        SfxMsgAttachFile( const String& rFile, const String& rName)
                            : aName( rName ), aFile( rFile ) {}

                        SfxMsgAttachFile( const SfxMsgAttachFile& rRec )
                            : aName( rRec.aName), aFile( rRec.aFile ) {}
};

enum SfxMsgReceiverRole
{
    MSG_RECEIVER_TO = 0,
    MSG_RECEIVER_CC,
    MSG_RECEIVER_BCC,
    MSG_RECEIVER_NEWSGROUP
};

struct SfxMsgReceiver {
    String              aName;
    SfxMsgReceiverRole  eRole;

    int                 operator==( const SfxMsgReceiver& rRec ) const
                        {
                            if( aName == rRec.aName && eRole == rRec.eRole )
                                return sal_True;
                            return sal_False;
                        }

                        SfxMsgReceiver( const String& rName, SfxMsgReceiverRole _eRole )
                            : aName( rName ), eRole( _eRole ) {}

                        SfxMsgReceiver( const SfxMsgReceiver& rRec )
                            : aName( rRec.aName), eRole( rRec.eRole ) {}
};

// ------------------------------------------------------------------------

class SfxMsgReceiverList_Impl : public List
{
    sal_uIntPtr             nRef;
                    ~SfxMsgReceiverList_Impl();
    SfxMsgReceiverList_Impl& operator=( const SfxMsgReceiverList_Impl&); //n.i.
public:
                    SfxMsgReceiverList_Impl();
                    SfxMsgReceiverList_Impl(const SfxMsgReceiverList_Impl&);

    void            Load( SvStream& );
    void            Store( SvStream& ) const;
    void            IncRef() { nRef++; }
    void            DecRef() { nRef--; if( !nRef ) delete this; }
    sal_uIntPtr         GetRefCount() const { return nRef; }
    int             operator==( const SfxMsgReceiverList_Impl& ) const;
};

class SfxMsgReceiverListItem : public SfxPoolItem
{
    void                    Disconnect();
protected:
    SfxMsgReceiverList_Impl* pImp;
public:
                            SfxMsgReceiverListItem();
                            SfxMsgReceiverListItem( sal_uInt16 nWhich );
                            SfxMsgReceiverListItem( sal_uInt16 nWhich, SvStream& rStream );
                            SfxMsgReceiverListItem( const SfxMsgReceiverListItem& rItem );
                            ~SfxMsgReceiverListItem();

    virtual int             operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText ) const;

    sal_uInt16                  Count() const;
    SfxMsgReceiver*         GetObject( sal_uInt16 nPos );
    void                    Remove( sal_uInt16 nPos );
    void                    Add( const SfxMsgReceiver& );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream &, sal_uInt16 nVersion ) const;
    virtual SvStream&       Store( SvStream &, sal_uInt16 nItemVersion ) const;
};


// ------------------------------------------------------------------------

class SfxMsgAttachFileList_Impl : public List
{
    sal_uIntPtr             nRef;
                    ~SfxMsgAttachFileList_Impl();
    SfxMsgAttachFileList_Impl& operator=( const SfxMsgAttachFileList_Impl&); //n.i.

public:
                    SfxMsgAttachFileList_Impl();
                    SfxMsgAttachFileList_Impl(const SfxMsgAttachFileList_Impl&);

    int             operator==( const SfxMsgAttachFileList_Impl& rRec ) const;
    SfxMsgAttachFile* GetReceiver(sal_uIntPtr nPos) { return (SfxMsgAttachFile*)List::GetObject(nPos); }
    void            Load( SvStream& );
    void            Store( SvStream& ) const;
    void            IncRef() { nRef++; }
    void            DecRef() { nRef--; if( !nRef ) delete this; }
    sal_uIntPtr         GetRefCount() const { return nRef; }
};

class SfxMsgAttachFileListItem : public SfxPoolItem
{
protected:
    SfxMsgAttachFileList_Impl* pImp;
    void                    Disconnect();
public:
                            SfxMsgAttachFileListItem();
                            SfxMsgAttachFileListItem( sal_uInt16 nWhich );
                            SfxMsgAttachFileListItem( sal_uInt16 nWhich, SvStream& rStream );
                            SfxMsgAttachFileListItem( const SfxMsgAttachFileListItem& rItem );
                            ~SfxMsgAttachFileListItem();

    virtual int             operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText ) const;

    sal_uInt16                  Count() const;
    SfxMsgAttachFile*       GetObject( sal_uInt16 nPos );
    void                    Remove( sal_uInt16 nPos );
    void                    Add( const SfxMsgAttachFile& );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream &, sal_uInt16 nVersion ) const;
    virtual SvStream&       Store( SvStream &, sal_uInt16 nItemVersion ) const;
};


//=========================================================================

enum SfxMsgPriority
{
    MSG_PRIORITY_LOW = 0,
    MSG_PRIORITY_NORMAL,
    MSG_PRIORITY_URGENT
};
#define SFXMSGPRIORITYCOUNT 3

class SfxMsgPriorityItem : public SfxEnumItem
{
public:
    SfxMsgPriorityItem( sal_uInt16 nWhich, SfxMsgPriority = MSG_PRIORITY_NORMAL);

    virtual SfxPoolItem*        Clone( SfxItemPool* pPool=0 ) const;
    virtual SfxPoolItem*        Create( SvStream&, sal_uInt16 ) const;
    virtual SvStream&           Store( SvStream&, sal_uInt16 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePresentation,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresentationMetric,
                                    String &rText ) const;
    virtual sal_uInt16              GetValueCount() const;
    virtual String              GetValueTextByPos( sal_uInt16 nPos ) const;

    inline SfxMsgPriorityItem& operator=(const SfxMsgPriorityItem& rPrio)
    {
        SetValue( rPrio.GetValue() );
        return *this;
    }
};


#endif
