package common;

$REFRESH_TIME = 5;

sub File_read
{
    $sFilename = @_[ 0 ];

    @aFileContentList = "";
    open( F_CURRPIC, "<" . $sFilename ) || "Could not open file " . $sFilename . " !<BR>\n";
    while( <F_CURRPIC> )
    {
        push( @aFileContentList, $_ );
    }
    close( F_CURRPIC );

    return @aFileContentList;
}   ##File_read


sub HTTP_getRequest
{
    # post- or get- method ?
    if( $ENV{ 'REQUEST_METHOD' } eq 'GET' )
    {
        # get parameters from querystring (get)
        $sRequest = $ENV{ 'QUERY_STRING' }
    }
    else
    {
        # get parameters from stdin (post)
        read( STDIN, $sRequest, $ENV{ 'CONTENT_LENGTH' } );
    }
    # process parameters
    @aRequestList = split( /&/, $sRequest );
    foreach $Feld ( @aRequestList )
    {
        ( $name, $sValue ) = split( /=/, $Feld );
        $sValue =~ tr/+/ /;
        $sValue =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
        $sValue =~ s/<!--(.|\n)*-->//g;
        $aRequestMap{ $name } = $sValue;
    }

    return %aRequestMap;
}   ##HTTP_getRequest

1;