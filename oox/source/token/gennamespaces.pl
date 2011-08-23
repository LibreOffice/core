$ARGV0 = shift @ARGV;
$ARGV1 = shift @ARGV;

open ( NAMESPACES, $ARGV0 ) || die "can't open namespaces file: $!";


open ( HXX, ">$ARGV1" ) or die "can't open namespaces.hxx file: $!";

print ( HXX "#ifndef OOX_NAMESPACES_HXX\n" );
print ( HXX "#define OOX_NAMESPACES_HXX\n\n" );
print ( HXX "#include <sal/types.h>\n\n" );
print ( HXX "namespace oox {\n\n" );

$group = 0;
$i = 1;
while ( <NAMESPACES> )
{
    chomp( $_ );
    $_ =~ s/\s*//g;
    if ( $_ =~ m/^$/ )
    {
        # Start a new group
        print ( HXX "\n" );
        $i = 0;
        $group++;
    } 
    elsif ( $_ =~ m/^[^#]/ )
    {
        # Neither an empty line nor a comment
        $_ =~ /^[a-zA-Z0-9-_]+$/ or die "Invalid namespace token $_";
        $id = "NMSP_$_";
        $id =~ s/-/_/g;
        $no = $group*10 + $i;
        print ( HXX "const sal_Int32 $id = $no << 16;\n" );
        ++$i;
    }
}
close ( NAMESPACES );

print ( HXX "\nconst sal_Int32 TOKEN_MASK                  = SAL_MAX_UINT16;\n" );
print ( HXX "const sal_Int32 NMSP_MASK                   = SAL_MAX_INT16 << 16;\n" );

print ( HXX "/** Returns the token identifier of the passed element without namespace. */\n" );
print ( HXX "inline sal_Int32 getToken( sal_Int32 nElement ) { return nElement & TOKEN_MASK; }\n\n" );

print ( HXX "/** Returns the namespace of the passed element without token identifier. */\n" );
print ( HXX "inline sal_Int32 getNamespace( sal_Int32 nElement ) { return nElement & NMSP_MASK; }\n\n" );

print ( HXX "// defines for tokens with specific namespaces, can be used in switch/cases\n\n" );
print ( HXX "#define A_TOKEN( token )            (::oox::NMSP_DRAWINGML | XML_##token)\n" );
print ( HXX "#define AX_TOKEN( token )           (::oox::NMSP_AX | XML_##token)\n" );
print ( HXX "#define C_TOKEN( token )            (::oox::NMSP_CHART | XML_##token)\n" );
print ( HXX "#define CDR_TOKEN( token )          (::oox::NMSP_CDR | XML_##token)\n" );
print ( HXX "#define O_TOKEN( token )            (::oox::NMSP_OFFICE | XML_##token)\n" );
print ( HXX "#define PPT_TOKEN( token )          (::oox::NMSP_PPT | XML_##token)\n" );
print ( HXX "#define R_TOKEN( token )            (::oox::NMSP_RELATIONSHIPS | XML_##token)\n" );
print ( HXX "#define VML_TOKEN( token )          (::oox::NMSP_VML | XML_##token)\n" );
print ( HXX "#define VMLX_TOKEN( token )         (::oox::NMSP_VML_XLS | XML_##token)\n" );
print ( HXX "#define XDR_TOKEN( token )          (::oox::NMSP_XDR | XML_##token)\n" );
print ( HXX "#define XLS_TOKEN( token )          (::oox::NMSP_XLS | XML_##token)\n" );
print ( HXX "#define XM_TOKEN( token )           (::oox::NMSP_XM | XML_##token)\n" );
print ( HXX "#define XML_TOKEN( token )          (::oox::NMSP_XML | XML_##token)\n" );


print ( HXX "} // namespace oox\n\n" );
print ( HXX "#endif // OOX_NAMESPACES_HXX\n" );
