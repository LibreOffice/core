#include "EventManager.hxx"
#include "SymbolLoader.hxx"

typedef struct libvlc_event_t
{
    int   type;
    void *p_obj;
    union
    {
        struct
        {
            void *p;
        } media_meta_changed;
    } u;
} libvlc_event_t;

typedef int libvlc_event_type_t;
typedef void ( *libvlc_callback_t ) ( const struct libvlc_event_t *, void * );

namespace VLC
{
    namespace
    {
        enum libvlc_event_e {
            libvlc_MediaMetaChanged=0,
            libvlc_MediaSubItemAdded,
            libvlc_MediaDurationChanged,
            libvlc_MediaParsedChanged,
            libvlc_MediaFreed,
            libvlc_MediaStateChanged,

            libvlc_MediaPlayerMediaChanged=0x100,
            libvlc_MediaPlayerNothingSpecial,
            libvlc_MediaPlayerOpening,
            libvlc_MediaPlayerBuffering,
            libvlc_MediaPlayerPlaying,
            libvlc_MediaPlayerPaused,
            libvlc_MediaPlayerStopped,
            libvlc_MediaPlayerForward,
            libvlc_MediaPlayerBackward,
            libvlc_MediaPlayerEndReached,
            libvlc_MediaPlayerEncounteredError,
            libvlc_MediaPlayerTimeChanged,
            libvlc_MediaPlayerPositionChanged,
            libvlc_MediaPlayerSeekableChanged,
            libvlc_MediaPlayerPausableChanged,
            libvlc_MediaPlayerTitleChanged,
            libvlc_MediaPlayerSnapshotTaken,
            libvlc_MediaPlayerLengthChanged,
            libvlc_MediaPlayerVout,

            libvlc_MediaListItemAdded=0x200,
            libvlc_MediaListWillAddItem,
            libvlc_MediaListItemDeleted,
            libvlc_MediaListWillDeleteItem,

            libvlc_MediaListViewItemAdded=0x300,
            libvlc_MediaListViewWillAddItem,
            libvlc_MediaListViewItemDeleted,
            libvlc_MediaListViewWillDeleteItem,

            libvlc_MediaListPlayerPlayed=0x400,
            libvlc_MediaListPlayerNextItemSet,
            libvlc_MediaListPlayerStopped,

            libvlc_MediaDiscovererStarted=0x500,
            libvlc_MediaDiscovererEnded,

            libvlc_VlmMediaAdded=0x600,
            libvlc_VlmMediaRemoved,
            libvlc_VlmMediaChanged,
            libvlc_VlmMediaInstanceStarted,
            libvlc_VlmMediaInstanceStopped,
            libvlc_VlmMediaInstanceStatusInit,
            libvlc_VlmMediaInstanceStatusOpening,
            libvlc_VlmMediaInstanceStatusPlaying,
            libvlc_VlmMediaInstanceStatusPause,
            libvlc_VlmMediaInstanceStatusEnd,
            libvlc_VlmMediaInstanceStatusError
        };

        libvlc_event_manager_t* ( *libvlc_media_player_event_manager ) ( libvlc_media_player_t *p_mi );
        int ( *libvlc_event_attach ) ( libvlc_event_manager_t *p_event_manager,
                                       libvlc_event_type_t i_event_type,
                                       libvlc_callback_t f_callback,
                                       void *user_data );
        void ( *libvlc_event_detach ) ( libvlc_event_manager_t *p_event_manager,
                                        libvlc_event_type_t i_event_type,
                                        libvlc_callback_t f_callback,
                                        void *p_user_data );

        ApiMap VLC_EVENT_MANAGER_API[] =
        {
            SYM_MAP( libvlc_media_player_event_manager ),
            SYM_MAP( libvlc_event_attach ),
            SYM_MAP( libvlc_event_detach )
        };
    }

void EventManagerEventHandler( const libvlc_event_t *event, void *pData )
{
    EventManager *instance = static_cast<EventManager*>( pData );
    switch ( event->type )
    {
    case libvlc_MediaPlayerPaused:
        instance->mOnPaused();
        break;
    case libvlc_MediaPlayerEndReached:
        instance->mOnEndReached();
        break;
    }
}

EventManager::EventManager(VLC::Player& player)
{
    InitApiMap( VLC_EVENT_MANAGER_API );
    mManager = libvlc_media_player_event_manager( player );
}

EventManager::~EventManager()
{
}

void EventManager::registerSignal(int signal, const Callback& callback)
{
    if (callback.empty())
        libvlc_event_detach( mManager, signal, EventManagerEventHandler, this );
    else
        libvlc_event_attach( mManager, signal, EventManagerEventHandler, this );
}

void EventManager::onPaused(const EventManager::Callback& callback)
{
    mOnPaused = callback;
    registerSignal(libvlc_MediaPlayerPaused, callback);
}

void EventManager::onEndReached(const Callback& callback)
{
    mOnEndReached = callback;
    registerSignal(libvlc_MediaPlayerEndReached, callback);
}

}