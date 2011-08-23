BEGIN {
	ExitRet = 0
	outfiles = 4
	if ( ARGC != (outfiles+2) ) {
		printf( "Syntax: AWK ... Input.RC IconBitmapCursorOut.RC Output2.RC " )
		printf( "Output3.RC Output4.RC\n" )
		ExitRet = 1
		exit ExitRet
	}
	printf( "splitting %s => %s, %s, %s, %s\n", ARGV[1], ARGV[2], ARGV[3], ARGV[4], ARGV[5] )
	IcoBitCu = ARGV[2]
	rc2 = ARGV[3]
	rc3 = ARGV[4]
	rc4 = ARGV[5]
	printf("") >IcoBitCu
	printf("") >rc2
	printf("") >rc3
	printf("") >rc4
	ARGC -= outfiles
	out = 2
}

{
	if ( $2 == "ICON" || $2 == "BITMAP" || $2 == "CURSOR" ) {
		print >>IcoBitCu
		if ( $2 == "ICON" )
			printf("i")
		else if ( $2 == "BITMAP" )
			printf("b")
		else if ( $2 == "CURSOR" )
			printf("c")
		else
			printf("?")
	}
	else {
		if ( 2 == out )
			print >>rc2
		else if ( 3 == out )
			print >>rc3
		else if ( 4 == out )
			print >>rc4
		if ( "END" == $0 ) {
			printf(".")
			out++
			if ( out > outfiles )
				out = 2
		}
	}
}

END {
	if ( ExitRet == 0 )
		printf(" :-)\n")
	else
		printf(" :-(\n")
	exit ExitRet
}
