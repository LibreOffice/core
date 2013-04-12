function Receiver() {

    var mSlideshow;

    this.parseMessage = function( aMessage ) {
        console.log( "Received message " + aMessage[0] );
        switch ( aMessage[0] ) {
            case "slideshow_started":
                mSlideshow = new Slideshow();
                var aSlideShowlength = aMessage[1];
                var aCurrentSlide = aMessage[2];
                mSlideshow.setLength( aSlideShowlength );
                mSlideshow.setCurrentSlide( aCurrentSlide );
                // TODO: notify listeners
                break;
            case "slideshow_finished":
                // TODO: notify listeners
                mSlideshow = '';
                break;
            default:
                if ( !mSlideshow ) {
                    break;
                }
                var mSlide = aMessage[1];
                // TODO: notify listeners
                switch ( aMessage[0] ) {
                    case "slide_updated":
                        mSlideshow.setCurrentSlide( mSlide );
                        updatePreview( mSlideshow );
                    case "slide_notes":
                        mSlideshow.putNotes( mSlide, aMessage[2] );
                        break;
                    case "slide_preview":
                        mSlideshow.putPreview( mSlide, aMessage[2] );
                        break;
                }
                break;
        }
    }

    this.getSlideshow = function() {
        return mSlideshow;
    }


}