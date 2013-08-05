#include <rtl/ustring.h>

#include "Player.hxx"
#include "Media.hxx"
#include "SymbolLoader.hxx"

struct libvlc_media_t;
namespace VLC
{
    namespace
    {
        typedef int64_t libvlc_time_t;

        libvlc_media_player_t * ( *libvlc_media_player_new_from_media ) ( libvlc_media_t *p_md );
        void ( *libvlc_media_player_release ) ( libvlc_media_player_t *p_mi );
        int ( *libvlc_media_player_play ) ( libvlc_media_player_t *p_mi );
        void ( *libvlc_media_player_pause ) ( libvlc_media_player_t *p_mi );
        int ( *libvlc_media_player_is_playing ) ( libvlc_media_player_t *p_mi );
        void ( *libvlc_media_player_stop ) ( libvlc_media_player_t *p_mi );
        void ( *libvlc_media_player_set_time ) ( libvlc_media_player_t *p_mi, libvlc_time_t i_time );
        libvlc_time_t ( *libvlc_media_player_get_time ) ( libvlc_media_player_t *p_mi );
        libvlc_time_t ( *libvlc_media_player_get_length ) ( libvlc_media_player_t *p_mi );
        float ( *libvlc_media_player_get_rate )( libvlc_media_player_t *p_mi );
        int ( *libvlc_audio_set_volume ) ( libvlc_media_player_t *p_mi, int i_volume );
        int ( *libvlc_audio_get_volume ) ( libvlc_media_player_t *p_mi );
        int ( *libvlc_audio_get_mute ) ( libvlc_media_player_t *p_mi );
        void ( *libvlc_audio_set_mute ) ( libvlc_media_player_t *p_mi, int status );
        int ( *libvlc_video_take_snapshot ) ( libvlc_media_player_t *p_mi, unsigned num,
                                const char *psz_filepath, unsigned int i_width,
                                unsigned int i_height );
        void ( *libvlc_media_player_set_xwindow ) ( libvlc_media_player_t *p_mi, uint32_t drawable );
        unsigned ( *libvlc_media_player_has_vout ) ( libvlc_media_player_t *p_mi );

        ApiMap VLC_PLAYER_API[] =
        {
            SYM_MAP( libvlc_media_player_new_from_media ),
            SYM_MAP( libvlc_media_player_release ),
            SYM_MAP( libvlc_media_player_play ),
            SYM_MAP( libvlc_media_player_pause ),
            SYM_MAP( libvlc_media_player_is_playing ),
            SYM_MAP( libvlc_media_player_stop ),
            SYM_MAP( libvlc_media_player_set_time ),
            SYM_MAP( libvlc_media_player_get_time ),
            SYM_MAP( libvlc_media_player_get_length ),
            SYM_MAP( libvlc_media_player_get_rate ),
            SYM_MAP( libvlc_audio_set_volume ),
            SYM_MAP( libvlc_audio_get_volume ),
            SYM_MAP( libvlc_audio_set_mute ),
            SYM_MAP( libvlc_audio_get_mute ),
            SYM_MAP( libvlc_video_take_snapshot ),
            SYM_MAP( libvlc_media_player_set_xwindow ),
            SYM_MAP( libvlc_media_player_has_vout )
        };
    }

    Player::Player(Media& media)
    {
        InitApiMap(VLC_PLAYER_API);
        mPlayer = libvlc_media_player_new_from_media( media );
    }

    Player::~Player()
    {
        libvlc_media_player_release( mPlayer );
    }

    bool Player::play()
    {
        return libvlc_media_player_play( mPlayer );
    }

    void Player::pause()
    {
        libvlc_media_player_pause( mPlayer );
    }

    void Player::stop()
    {
        libvlc_media_player_stop( mPlayer );
    }

    void Player::setTime( int time )
    {
        libvlc_media_player_set_time( mPlayer, time );
    }

    int Player::getTime() const
    {
        return libvlc_media_player_get_time( mPlayer );
    }

    bool Player::isPlaying() const
    {
        return libvlc_media_player_is_playing( mPlayer ) == 1;
    }

    int Player::getLength() const
    {
        return libvlc_media_player_get_length( mPlayer );
    }

    float Player::getRate() const
    {
        return libvlc_media_player_get_rate( mPlayer );
    }

    void Player::setVolume( int volume )
    {
        libvlc_audio_set_volume( mPlayer, volume );
    }

    int Player::getVolume() const
    {
        return libvlc_audio_get_volume( mPlayer );
    }

    void Player::setMute( bool mute)
    {
        libvlc_audio_set_mute( mPlayer, mute );
    }

    bool Player::getMute() const
    {
        return libvlc_audio_get_mute( mPlayer );
    }

    void Player::setXWindow( int id )
    {
        libvlc_media_player_set_xwindow( mPlayer, id );
    }

    void Player::takeSnapshot(const rtl::OUString& file)
    {
        rtl::OString dest;
        file.convertToString( &dest, RTL_TEXTENCODING_UTF8, 0 );
        libvlc_video_take_snapshot( mPlayer, 0, dest.getStr(), 0, 0 );
    }

    bool Player::hasVout() const
    {
        return libvlc_media_player_has_vout( mPlayer );
    }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */