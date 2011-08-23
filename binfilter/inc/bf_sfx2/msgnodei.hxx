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

#ifndef _MSGNODEI_HXX
#define _MSGNODEI_HXX


#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
namespace binfilter {
struct SfxMsgAttachFile {
    String	aFile;
    String	aName;

    int 				operator==( const SfxMsgAttachFile& rRec ) const
                        {
                            if( aName == rRec.aName && aFile == rRec.aFile )
                                return TRUE;
                            return FALSE;
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
    String				aName;
    SfxMsgReceiverRole  eRole;

    int 				operator==( const SfxMsgReceiver& rRec ) const
                        {
                            if( aName == rRec.aName && eRole == rRec.eRole )
                                return TRUE;
                            return FALSE;
                        }

                        SfxMsgReceiver( const String& rName, SfxMsgReceiverRole _eRole )
                            : aName( rName ), eRole( _eRole ) {}

                        SfxMsgReceiver( const SfxMsgReceiver& rRec )
                            : aName( rRec.aName), eRole( rRec.eRole ) {}
};

// ------------------------------------------------------------------------

class SfxMsgReceiverList_Impl : public List
{
    ULONG 			nRef;
                    ~SfxMsgReceiverList_Impl();
    SfxMsgReceiverList_Impl& operator=( const SfxMsgReceiverList_Impl&); //n.i.
public:
                    SfxMsgReceiverList_Impl();
                    SfxMsgReceiverList_Impl(const SfxMsgReceiverList_Impl&);

    void			Load( SvStream& );
    void			Store( SvStream& ) const;
    void 			IncRef() { nRef++; }
    void 			DecRef() { nRef--; if( !nRef ) delete this; }
    ULONG			GetRefCount() const { return nRef; }
    int 			operator==( const SfxMsgReceiverList_Impl& ) const;
};

class SfxMsgReceiverListItem : public SfxPoolItem
{
    void					Disconnect();
protected:
    SfxMsgReceiverList_Impl* pImp;
public:
                            TYPEINFO();

                            SfxMsgReceiverListItem();
                            SfxMsgReceiverListItem( USHORT nWhich );
                            SfxMsgReceiverListItem( USHORT nWhich, SvStream& rStream );
                            SfxMsgReceiverListItem( const SfxMsgReceiverListItem& rItem );
                            ~SfxMsgReceiverListItem();

    virtual int 			operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText ) const;

    USHORT					Count() const;
    SfxMsgReceiver*			GetObject( USHORT nPos );
    void					Remove( USHORT nPos );
    void					Add( const SfxMsgReceiver& );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	Create( SvStream &, USHORT nVersion ) const;
    virtual SvStream&		Store( SvStream &, USHORT nItemVersion ) const;
};


// ------------------------------------------------------------------------

class SfxMsgAttachFileList_Impl : public List
{
    ULONG 			nRef;
                    ~SfxMsgAttachFileList_Impl();
    SfxMsgAttachFileList_Impl& operator=( const SfxMsgAttachFileList_Impl&); //n.i.

public:
                    SfxMsgAttachFileList_Impl();
                    SfxMsgAttachFileList_Impl(const SfxMsgAttachFileList_Impl&);

    int				operator==( const SfxMsgAttachFileList_Impl& rRec ) const;
    SfxMsgAttachFile* GetReceiver(ULONG nPos) { return (SfxMsgAttachFile*)List::GetObject(nPos); }
    void			Load( SvStream& );
    void			Store( SvStream& ) const;
    void 			IncRef() { nRef++; }
    void 			DecRef() { nRef--; if( !nRef ) delete this; }
    ULONG			GetRefCount() const { return nRef; }
};

class SfxMsgAttachFileListItem : public SfxPoolItem
{
protected:
    SfxMsgAttachFileList_Impl* pImp;
    void					Disconnect();
public:
                            TYPEINFO();

                            SfxMsgAttachFileListItem();
                            SfxMsgAttachFileListItem( USHORT nWhich );
                            SfxMsgAttachFileListItem( USHORT nWhich, SvStream& rStream );
                            SfxMsgAttachFileListItem( const SfxMsgAttachFileListItem& rItem );
                            ~SfxMsgAttachFileListItem();

    virtual int 			operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText ) const;

    USHORT					Count() const;
    SfxMsgAttachFile*		GetObject( USHORT nPos );
    void					Remove( USHORT nPos );
    void					Add( const SfxMsgAttachFile& );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	Create( SvStream &, USHORT nVersion ) const;
    virtual SvStream&		Store( SvStream &, USHORT nItemVersion ) const;
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
    TYPEINFO();

    SfxMsgPriorityItem( USHORT nWhich, SfxMsgPriority = MSG_PRIORITY_NORMAL);

    virtual SfxPoolItem*	 	Clone( SfxItemPool* pPool=0 ) const;
    virtual SfxPoolItem*	 	Create( SvStream&, USHORT ) const;
    virtual SvStream&		 	Store( SvStream&, USHORT ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePresentation,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresentationMetric,
                                    String &rText ) const;
    virtual USHORT			 	GetValueCount() const;
    virtual String			 	GetValueTextByPos( USHORT nPos ) const;

    inline SfxMsgPriorityItem& operator=(const SfxMsgPriorityItem& rPrio)
    {
        SetValue( rPrio.GetValue() );
        return *this;
    }
};


}//end of namespace binfilter
#endif
