/* NOT copyright by SoftQuad Inc. -- msb, 1988 */
#ifndef lint
static char *SQ_SccsId = "@(#)mtest3.c  1.2 88/08/25";
#endif
#include <stdio.h>
/*
** looptest.c -- intensive allocator tester
**
** Usage:  looptest
**
** History:
**  4-Feb-1987 rtech!daveb
*/

# ifdef SYS5
# define random rand
# else
# include <sys/vadvise.h>
# endif

# include <stdio.h>
# include <signal.h>
# include <setjmp.h>

# define MAXITER    1000000     /* main loop iterations */
# define MAXOBJS    1000        /* objects in pool */
# define BIGOBJ     90000       /* max size of a big object */
# define TINYOBJ    80      /* max size of a small object */
# define BIGMOD     100     /* 1 in BIGMOD is a BIGOBJ */
# define STATMOD    10000       /* interation interval for status */

main( argc, argv )
int argc;
char **argv;
{
    register int **objs;        /* array of objects */
    register int *sizes;        /* array of object sizes */
    register int n;         /* iteration counter */
    register int i;         /* object index */
    register int size;      /* object size */
    register int r;         /* random number */

    int objmax;         /* max size this iteration */
    int cnt;            /* number of allocated objects */
    int nm = 0;         /* number of mallocs */
    int nre = 0;            /* number of reallocs */
    int nal;            /* number of allocated objects */
    int nfre;           /* number of free list objects */
    long alm;           /* memory in allocated objects */
    long frem;          /* memory in free list */
    long startsize;         /* size at loop start */
    long endsize;           /* size at loop exit */
    long maxiter = 0;       /* real max # iterations */

    extern char end;        /* memory before heap */
    char *calloc();
    char *malloc();
    char *sbrk();
    long atol();

# ifndef SYS5
    /* your milage may vary... */
    vadvise( VA_ANOM );
# endif

    if (argc > 1)
        maxiter = atol (argv[1]);
    if (maxiter <= 0)
        maxiter = MAXITER;

    printf("MAXITER %d MAXOBJS %d ", maxiter, MAXOBJS );
    printf("BIGOBJ %d, TINYOBJ %d, nbig/ntiny 1/%d\n",
    BIGOBJ, TINYOBJ, BIGMOD );
    fflush( stdout );

    if( NULL == (objs = (int **)calloc( MAXOBJS, sizeof( *objs ) ) ) )
    {
        fprintf(stderr, "Can't allocate memory for objs array\n");
        exit(1);
    }

    if( NULL == ( sizes = (int *)calloc( MAXOBJS, sizeof( *sizes ) ) ) )
    {
        fprintf(stderr, "Can't allocate memory for sizes array\n");
        exit(1);
    }

    /* as per recent discussion on net.lang.c, calloc does not
    ** necessarily fill in NULL pointers...
    */
    for( i = 0; i < MAXOBJS; i++ )
        objs[ i ] = NULL;

    startsize = sbrk(0) - &end;
    printf( "Memory use at start: %d bytes\n", startsize );
    fflush(stdout);

    printf("Starting the test...\n");
    fflush(stdout);
    for( n = 0; n < maxiter ; n++ )
    {
        if( !(n % STATMOD) )
        {
            printf("%d iterations\n", n);
            fflush(stdout);
        }

        /* determine object of interst and it's size */

        r = random();
        objmax = ( r % BIGMOD ) ? TINYOBJ : BIGOBJ;
        size = r % objmax;
        i = r % (MAXOBJS - 1);

        /* either replace the object of get a new one */

        if( objs[ i ] == NULL )
        {
            objs[ i ] = (int *)malloc( size );
            nm++;
        }
        else
        {
            /* don't keep bigger objects around */
            if( size > sizes[ i ] )
            {
                objs[ i ] = (int *)realloc( objs[ i ], size );
                nre++;
            }
            else
            {
                free( objs[ i ] );
                objs[ i ] = (int *)malloc( size );
                nm++;
            }
        }

        sizes[ i ] = size;
        if( objs[ i ] == NULL )
        {
            printf("\nCouldn't allocate %d byte object!\n",
                size );
            break;
        }
    } /* for() */

    printf( "\n" );
    cnt = 0;
    for( i = 0; i < MAXOBJS; i++ )
        if( objs[ i ] )
            cnt++;

    printf( "Did %d iterations, %d objects, %d mallocs, %d reallocs\n",
        n, cnt, nm, nre );
    printf( "Memory use at end: %d bytes\n", sbrk(0) - &end );
    fflush( stdout );

    /* free all the objects */
    for( i = 0; i < MAXOBJS; i++ )
        if( objs[ i ] != NULL )
            free( objs[ i ] );

    endsize = sbrk(0) - &end;
    printf( "Memory use after free: %d bytes\n", endsize );
    fflush( stdout );

    if( startsize != endsize )
        printf("startsize %d != endsize %d\n", startsize, endsize );

    free( objs );
    free( sizes );

    malloc_dump(2);
    exit( 0 );
}

