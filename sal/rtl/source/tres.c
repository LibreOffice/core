#include <stdio.h>
#include <rtl/tres.h>

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_TIMEE_H_
#include <osl/time.h>
#endif

 /* force an assertion on false state */
#define TST_BOOM(c, m)  OSL_ENSURE(c, m)


typedef struct _rtl_CmpState
{
    struct _rtl_CmpState*   m_next;
    struct _rtl_CmpState*   m_prev;

    sal_Bool                m_stat;
    rtl_String*             m_msg;

} rtl_CmpState;

typedef struct _rtl_FuncState
{
    struct _rtl_FuncState*  m_next;
    struct _rtl_FuncState*  m_prev;
    rtl_String*             m_name;
    sal_uInt32              m_flags;
    sal_uInt32              m_start;
    sal_uInt32              m_stop;
    struct _rtl_CmpState*   m_cmp;

} rtl_FuncState;



typedef struct _rtl_TestResult_Data
{
    rtl_TestResult_vtable*  m_funcs;
    void*                   m_externaldata;

    rtl_FuncState*   m_state;

} rtl_TestResult_Data;


 /**
  * internal helper functions
  */

 /* ...to create, link, unlink and destroy allocated memory */
rtl_FuncState* SAL_CALL rtl_tres_create_funcstate( const sal_Char* meth );
rtl_FuncState* SAL_CALL rtl_tres_link_funcstate( rtl_FuncState* ptr,
                                                    rtl_FuncState* plink );
rtl_FuncState* SAL_CALL rtl_tres_unlink_funcstate( rtl_FuncState* plink );
rtl_CmpState* SAL_CALL rtl_tres_create_cmpstate(
                                                sal_Bool state,
                                                const sal_Char* msg
                                                );
rtl_CmpState* SAL_CALL rtl_tres_link_cmpstate( rtl_CmpState* ptr,
                                                    rtl_CmpState* plink );
rtl_CmpState* SAL_CALL rtl_tres_unlink_cmpstate( rtl_CmpState* plink );
sal_uInt32 SAL_CALL rtl_tres_timer();
void SAL_CALL rtl_tres_destroy_funcstate( rtl_FuncState* pState_ );
void SAL_CALL rtl_tres_destroy_funcstates( rtl_FuncState* pState_ );
void SAL_CALL rtl_tres_destroy_cmpstates( rtl_CmpState* pState_ );
void SAL_CALL rtl_tres_destroy_cmpstate( rtl_CmpState* pState_ );


 /* set and clear single bits */
static void SAL_CALL rtl_tres_setbit( rtl_FuncState* pState_,
                                                          sal_uInt32 flag  );
static void SAL_CALL rtl_tres_clearbit( rtl_FuncState* pState_,
                                                          sal_uInt32 flag  );

 /**
  * forward declarations of concrete function implementations overloadable
  * and accessible via vtable
  */
static sal_Bool SAL_CALL rtl_tres_state(
                                        rtl_TestResult* pThis_,
                                        sal_Bool state,
                                        const sal_Char* msg,
                                        const sal_Char* sub,
                                        sal_Bool v
                                        );

static void SAL_CALL rtl_tres_end( rtl_TestResult* pThis_,
                                                        const sal_Char* msg );

static void SAL_CALL rtl_tres_sub( rtl_TestResult* pThis_,
                                                    const sal_Char* name );

static rtl_funcstate SAL_CALL rtl_tres_funcstate( rtl_TestResult* pThis_ );

static sal_Bool SAL_CALL rtl_tres_ispassed( rtl_TestResult* pThis_ );
static sal_Bool SAL_CALL rtl_tres_isok( rtl_TestResult* pThis_ );

static sal_Bool SAL_CALL rtl_tres_isbit( rtl_TestResult* pThis_,
                                                        sal_uInt32 flag );

static rtl_funcstate SAL_CALL rtl_tres_getnextfuncstate( rtl_funcstate );
static rtl_funcstate SAL_CALL rtl_tres_getprevfuncstate( rtl_funcstate );
static sal_uInt32 SAL_CALL rtl_tres_getflags( rtl_funcstate );
sal_uInt32 SAL_CALL rtl_tres_getstarttime( rtl_funcstate );
sal_uInt32 SAL_CALL rtl_tres_getstoptime( rtl_funcstate );
static rtl_cmpstate SAL_CALL rtl_tres_getcmpstate( rtl_funcstate );

static sal_Bool SAL_CALL rtl_tres_getstat( rtl_cmpstate );
rtl_String* SAL_CALL rtl_tres_getname( rtl_funcstate );
rtl_String* SAL_CALL rtl_tres_getmsg( rtl_cmpstate );
static rtl_cmpstate SAL_CALL rtl_tres_getnextcmpstate( rtl_cmpstate );


 /**
 * initialize vtable with function pointers
 */
static rtl_TestResult_vtable trVTable =
{
    sizeof(rtl_TestResult_vtable),
    rtl_tres_state,
    rtl_tres_end,
    rtl_tres_ispassed,
    rtl_tres_isok,
    rtl_tres_funcstate,
    rtl_tres_isbit,
    rtl_tres_getnextfuncstate,
    rtl_tres_getprevfuncstate,
    rtl_tres_getflags,
    rtl_tres_getname,
    rtl_tres_getstarttime,
    rtl_tres_getstoptime,
    rtl_tres_getcmpstate,
    rtl_tres_getstat,
    rtl_tres_getmsg,
    rtl_tres_getnextcmpstate
};

 /**
  * rtl_tres_create
  * create and initialize data struct for TestResult
  *
  * @param const sal_Char* meth = name of the method (entryname)
  * @param sal_uInt32 flags     = bitmap of comandline and status flags
  *
  * @return rtl_TestResult*     = pointer to a new allocated testresult struct
  */
rtl_TestResult* rtl_tres_create( const sal_Char* meth, sal_uInt32 flags )
{
    /* allocate memory for testresult data structure */
    rtl_TestResult_Data* pData = (rtl_TestResult_Data*) malloc( sizeof(
                                                    rtl_TestResult_Data ) );
    /* initialize members... */
    pData->m_funcs              = &trVTable;    /* ...vtableptr to vtbladr */
    pData->m_externaldata       = 0;            /* ...external data pointer */

    /* allocate memory for state structure and initialize members */
    pData->m_state              = rtl_tres_create_funcstate( meth );
    pData->m_state->m_flags     = flags;        /* ...option Bitmap */

    /* set OK flag initial */
    rtl_tres_setbit( pData->m_state, rtl_tres_Flag_OK );

    return (rtl_TestResult*)pData ;
}

/**
 * rtl_tres_create_funcstate
 * allocates and initializes a structure to represent the status of a test
 * entry or its substates
 *
 * @param const sal_Char* meth = the name of the method (entry or sub entry)
 *
 * @return rtl_FuncState* = pointer to a new allocated funcstate struct
 */
rtl_FuncState* SAL_CALL rtl_tres_create_funcstate( const sal_Char* meth )
{
    rtl_FuncState* pStat = 0;                   /* status structure */

    /* allocate memory for status structure */
    pStat = (rtl_FuncState*) malloc( sizeof( struct _rtl_FuncState ) );

    if ( pStat )
    {
        pStat->m_next  = pStat;                 /* init ptr to next struct */
        pStat->m_prev  = pStat;                 /* init ptr to prev struct */

        pStat->m_name  = 0;                     /* init name */
        pStat->m_flags = 0;                     /* init flags */
        pStat->m_start = rtl_tres_timer();      /* init start milliseconds */
        pStat->m_stop  = 0;                     /* init stop milliseconds */
        pStat->m_cmp   = 0;                     /* init ptr to msg struct */
        rtl_string_newFromStr( &pStat->m_name, meth );/* copy meth to name */

        /* set ok flag initially */
        rtl_tres_setbit(pStat, rtl_tres_Flag_OK);
    }

    return ( pStat );
}
 /**
  * rtl_tres_link_funcstate
  * link initialized funcstate structure to a circular double linked list
  *
  * @param rtl_FuncState* ptr   = pointer to a funcstate where to link in new
  * @param rtl_FuncState* plink = pointer to a funcstate to link in list
  *
  * @return rtl_FuncState*      = pointer to structure linked in new
  */
rtl_FuncState* SAL_CALL rtl_tres_link_funcstate( rtl_FuncState* ptr,
                                                        rtl_FuncState* plink )
{
    ptr->m_next->m_prev = plink;
    ptr->m_next->m_prev->m_next = ptr->m_next;
    ptr->m_next->m_prev->m_prev = ptr;
    ptr->m_next = plink;
    return ( plink );
}

 /**
  * rtl_tres_unlink_funcstate
  * unlink funcstate structure from a circular double linked list
  *
  * @param rtl_FuncState* plink = pointer to a funcstate to unlink from list
  *
  * @return rtl_FuncState*      = pointer to funcstate struct unlinked from
  *                               list
  */
rtl_FuncState* SAL_CALL rtl_tres_unlink_funcstate( rtl_FuncState* plink )
{
    plink->m_next->m_prev = plink->m_prev;
    plink->m_prev->m_next = plink->m_next;
    plink->m_next = plink;
    plink->m_prev = plink;
    return ( plink );
}

 /**
  * rtl_tres_link_cmpstate
  * link initialized cmpstate structure to a circular double linked list
  *
  * @param rtl_CmpState* ptr   = pointer to a cmpstate where to link in new
  * @param rtl_CmpState* plink = pointer to a cmpstate to link in list
  *
  * @return rtl_CmpState*      = pointer to cmpstate struct linked in new
  */
rtl_CmpState* SAL_CALL rtl_tres_link_cmpstate( rtl_CmpState* ptr,
                                                        rtl_CmpState* plink )
{
    ptr->m_next->m_prev = plink;
    ptr->m_next->m_prev->m_next = ptr->m_next;
    ptr->m_next->m_prev->m_prev = ptr;
    ptr->m_next = plink;
    return ( plink );
}
 /**
  * rtl_tres_unlink_cmpstate
  * unlink cmpstate structure from a circular double linked list
  *
  * @param rtl_CmpState* plink = pointer to a cmpstate to unlink from list
  *
  * @return rtl_CmpState*      = pointer to cmpstate struct unlinked from list
  */
rtl_CmpState* SAL_CALL rtl_tres_unlink_cmpstate( rtl_CmpState* plink )
{
    plink->m_next->m_prev = plink->m_prev;
    plink->m_prev->m_next = plink->m_next;
    plink->m_next = plink;
    plink->m_prev = plink;
    return ( plink );
}

 /**
  * rtl_tres_create_cmpstate
  * allocates and initializes a structure to represent the status of a test
  * comparison
  *
  * @param sal_Bool state   = compare state
  * @param sal_Char* msg    = message for logging and debug purposes
  *
  * @return rtl_CmpState*   = pointer to the new allocated struct
  */
rtl_CmpState* SAL_CALL rtl_tres_create_cmpstate(
                                                sal_Bool state,
                                                const sal_Char* msg
                                                )
{
    /* allocate memory for cmpstate struct */
    rtl_CmpState* pStat = (rtl_CmpState*) malloc( sizeof( rtl_CmpState ) );

    /* initialize if memory could be allocated */
    if ( pStat )
    {
        pStat->m_next   = pStat;                /* init next with this */
        pStat->m_prev   = pStat;                /* init prev with this */
        pStat->m_msg    = 0;
        pStat->m_stat   = state;                /* boolean state */
        rtl_string_newFromStr( &pStat->m_msg, msg ); /* copy message */
    }
    return ( pStat );
}

 /**
  * rtl_tres_destroy
  * free allocated memory of testresult data struct
  *
  * @param rtl_TestResult* pThis_ = ponter to a valid testresult struct
  */
void SAL_CALL rtl_tres_destroy( rtl_TestResult* pThis_ )
{
    /* cast to implementation representation structure */
    rtl_TestResult_Data* pData = (rtl_TestResult_Data*) pThis_;

    /* destroy all funcstates */
    if ( pData->m_state )
        rtl_tres_destroy_funcstates( pData->m_state );

    /* free allocted memory and reinitialize to zero */
    /* to be able to prevent dangling pointer access*/
    free( pData ); pData = 0;
}

 /**
  * rtl_tres_destroy_funcstates
  * free allocated memory occupied by the list of funcstate data structs
  * (iterates through next pointers)
  *
  * @param rtl_FuncState* pState_ = pointer to a valid funcstate struct
  */
void SAL_CALL rtl_tres_destroy_funcstates( rtl_FuncState* pState_ )
{
    rtl_FuncState* plink = pState_->m_next;
    while ( plink != plink->m_next )
    {
        rtl_tres_destroy_funcstate( rtl_tres_unlink_funcstate( plink ) );
        plink = pState_->m_next;
    }
    rtl_tres_destroy_funcstate( plink );
}

 /**
  * rtl_tres_destroy_cmpstates
  * free allocated memory occupied by the list of cmpstate data structs
  * (iterates through next pointers)
  *
  * @param rtl_CmpState* pState_ = pointer to a valid cmpstate struct
  */
void SAL_CALL rtl_tres_destroy_cmpstates( rtl_CmpState* pState_ )
{
    rtl_CmpState* plink = pState_->m_next;
    while ( plink != plink->m_next )
    {
        rtl_tres_destroy_cmpstate( rtl_tres_unlink_cmpstate( plink ) );
        plink = pState_->m_next;
    }
    rtl_tres_destroy_cmpstate( plink );
}


 /**
  * rtl_tres_destroy_funcstate
  * free allocated memory occupied by one funcstate and it's list
  * of cmpstate data structs
  *
  * @param rtl_FuncState* pState_ = pointer to a valid funcstate struct
  */
void SAL_CALL rtl_tres_destroy_funcstate( rtl_FuncState* pState_ )
{
    rtl_FuncState* plink = pState_;

    if ( plink->m_cmp )
        rtl_tres_destroy_cmpstates( plink->m_cmp );

    if ( plink->m_name )
    {
        rtl_string_release( plink->m_name );
        plink->m_name = 0;
    }
    plink->m_flags = 0;
    free( plink );
    plink = 0;
}

 /**
  * rtl_tres_destroy_cmpstate
  * free allocated memory of a cmpstate data struct
  *
  * @param rtl_CmpState* pState_ = pointer to cmpstate struct to destroy
  */
void SAL_CALL rtl_tres_destroy_cmpstate( rtl_CmpState* pState_ )
{

    rtl_CmpState* plink = pState_;

    if ( plink->m_msg )
    {
        rtl_string_release( plink->m_msg );
        plink->m_msg = 0;
    }
    free( plink );
    plink = 0;
}
 /**
 * central function to call in tests
 *
 * @param rtl_TestResult* pThis_    = self pointer to TestResult structure
 * @param sal_Bool state            = boolean result of statement comparison
 * @param const sal_Char* msg       = message for actual statementcomparison
 * @param const sal_Char* sub       = name of sub testfunction
 * @param sal_Bool v                = boolean verbose parameter
 *
 * @return sal_Bool                 = determines if statement comparison
 *                                    was positive or not
 */
static sal_Bool SAL_CALL rtl_tres_state(
                                        rtl_TestResult* pThis_,
                                        sal_Bool state,
                                        const sal_Char* msg,
                                        const sal_Char* sub,
                                        sal_Bool v
                                        )
{

    /* cast pointer to testresult data implementation struct*/
    rtl_TestResult_Data* pData = (rtl_TestResult_Data*)pThis_;

    /* initialize funcstate pointer with masterstate */
    rtl_FuncState* pFunc = pData->m_state;

    /* if substate required */
    if ( sub )
    {
        /* link new created function state to last item */
        pFunc = rtl_tres_link_funcstate( pFunc->m_prev,
                                        rtl_tres_create_funcstate( sub ) );

        /* indicate this state as substate */
        rtl_tres_setbit( pFunc, rtl_tres_Flag_SUB );

        /* indicate prvious state as passed if no masterstate */
        if ( pFunc->m_prev != pData->m_state )
            rtl_tres_setbit( pFunc->m_prev, rtl_tres_Flag_PASSED );
    }


    /* test failed */
    if( ! state )
    {
         /* determine if assertion should be thrown */
        if ( rtl_tres_isbit( pThis_, rtl_tres_Flag_BOOM ) )
        {
            /* if message available */
            if ( msg )
                TST_BOOM( state, msg );
            else
                TST_BOOM( state, "no msg available" );
        }

        /* clear this state ok flag and masterstate ok flag */
        rtl_tres_clearbit( pFunc, rtl_tres_Flag_OK );
        rtl_tres_clearbit( pData->m_state, rtl_tres_Flag_OK );
    }
    /* message available */
    if( msg )
    {
        /* append a new comparison state */
        if (! pFunc->m_cmp )
            pFunc->m_cmp = rtl_tres_create_cmpstate( state, msg );
        else
            rtl_tres_link_cmpstate( pFunc->m_cmp,
                            rtl_tres_create_cmpstate(state, msg ) );

        /* message to stderr required ? */
        if ( v || ( pFunc->m_next->m_flags & rtl_tres_Flag_VERBOSE ) )
            fprintf( stderr, "%s\n", msg );
    }

    pFunc->m_stop = rtl_tres_timer();
    return ( state );
}

 /**
  * rtl_tres_timer
  * function to get actual timevalue
  * this has to be replaced by a high resolution timer
  */
sal_uInt32 SAL_CALL rtl_tres_timer()
{
    sal_uInt32 val = 0;
    TimeValue* tmv = (TimeValue*)malloc( sizeof( TimeValue ) );
    osl_getSystemTime( tmv );
    val = tmv->Nanosec/1000L;
    free( tmv );
    return ( val );
}


static void SAL_CALL rtl_tres_end( rtl_TestResult* pThis_,
                                                        const sal_Char* msg )
{
    rtl_TestResult_Data* pData = (rtl_TestResult_Data*) pThis_;

    if( msg )
    {
        if (! pData->m_state->m_cmp )
            pData->m_state->m_cmp = rtl_tres_create_cmpstate( sal_True, msg );
        else
            rtl_tres_link_cmpstate( pData->m_state->m_cmp,
                            rtl_tres_create_cmpstate( sal_True, msg ) );
    }
    pData->m_state->m_prev->m_flags |= rtl_tres_Flag_PASSED;
    pData->m_state->m_flags |= rtl_tres_Flag_PASSED;
    pData->m_state->m_stop = rtl_tres_timer();
}


static sal_Bool SAL_CALL rtl_tres_ispassed( rtl_TestResult* pThis_ )
{
    return rtl_tres_isbit( pThis_, rtl_tres_Flag_PASSED );
}

static sal_Bool SAL_CALL rtl_tres_isok( rtl_TestResult* pThis_ )
{
    return rtl_tres_isbit( pThis_, rtl_tres_Flag_OK );
}
 /**
 * return pointer to funcstate structure
 */
static rtl_funcstate SAL_CALL rtl_tres_funcstate( rtl_TestResult* pThis_ )
{

    rtl_TestResult_Data* pThis = (rtl_TestResult_Data*) pThis_;
    return (rtl_funcstate)pThis->m_state;
}

 /**
 * determine if a flag is set or not
 */
static sal_Bool SAL_CALL rtl_tres_isbit( rtl_TestResult* pThis_,
                                                          sal_uInt32 flag  )
{

    return ( ( ((rtl_TestResult_Data*)pThis_)->m_state->m_flags & flag )
                                                                    == flag );
}
 /**
  * set one single bit
  */
static void SAL_CALL rtl_tres_setbit( rtl_FuncState* pState_,
                                                          sal_uInt32 flag  )
{
    pState_->m_flags |= flag;
}
 /**
  * clear one single bit
  */
static void SAL_CALL rtl_tres_clearbit( rtl_FuncState* pState_,
                                                          sal_uInt32 flag  )
{
    pState_->m_flags = pState_->m_flags & ( ~flag );
}

 /**
  * returns next pointer of passed funcstate structure
  */
rtl_funcstate SAL_CALL rtl_tres_getnextfuncstate( rtl_funcstate fstate )
{
    rtl_FuncState* fs = (rtl_FuncState*)fstate;
    return( (rtl_funcstate)fs->m_next );

}
 /**
  * returns previous pointer of passed funcstate structure
  */
rtl_funcstate SAL_CALL rtl_tres_getprevfuncstate( rtl_funcstate fstate )
{
    rtl_FuncState* fs = (rtl_FuncState*)fstate;
    return( (rtl_funcstate)fs->m_prev );

}
 /**
  * returns flag value of passed funcstate structure
  */
sal_uInt32 SAL_CALL rtl_tres_getflags( rtl_funcstate fstate )
{
    rtl_FuncState* fs = (rtl_FuncState*)fstate;
    return( fs->m_flags );
}
 /**
  * returns name of passed funcstate structure
  */
rtl_String* SAL_CALL rtl_tres_getname( rtl_funcstate fstate )
{
    rtl_FuncState* fs = (rtl_FuncState*)fstate;
    return( fs->m_name );
}
 /**
  * returns starttime of passed funcstate structure
  */
sal_uInt32 SAL_CALL rtl_tres_getstarttime( rtl_funcstate fstate )
{
    rtl_FuncState* fs = (rtl_FuncState*)fstate;
    return( fs->m_start );
}

 /**
  * returns stoptime of passed funcstate structure
  */
sal_uInt32 SAL_CALL rtl_tres_getstoptime( rtl_funcstate fstate )
{
    rtl_FuncState* fs = (rtl_FuncState*)fstate;
    return( fs->m_stop );
}

 /**
  * returns pointer to cmpstate of passed funcstate structure
  */
rtl_cmpstate SAL_CALL rtl_tres_getcmpstate( rtl_funcstate fstate)
{
    rtl_FuncState* fs = (rtl_FuncState*)fstate;
    return( (rtl_cmpstate)fs->m_cmp );

}
 /**
  * returns boolean state of passed cmpstate structure
  */
sal_Bool SAL_CALL rtl_tres_getstat( rtl_cmpstate cstate)
{
    rtl_CmpState* cs = (rtl_CmpState*)cstate;
    return( cs->m_stat );
}
 /**
  * returns message of passed cmpstate structure
  */
rtl_String* SAL_CALL rtl_tres_getmsg( rtl_cmpstate cstate)
{
    rtl_CmpState* cs = (rtl_CmpState*)cstate;
    return( cs->m_msg );
}
 /**
  * returns next pointer of passed cmpstate structure
  */
rtl_cmpstate SAL_CALL rtl_tres_getnextcmpstate( rtl_cmpstate cstate)
{
    rtl_CmpState* cs = (rtl_CmpState*)cstate;
    return( (rtl_cmpstate)cs->m_next );
}

/*
// <method_logPrintf>
//inline void logPrintf ( const sal_Bool   bTestCaseState,
//                            const char      *pFormatStr, ...
//                            )
//{
//    if( m_pFunctions && m_pFunctions->pLogPrintf )
//    {
//        va_list   vArgumentList;
//        va_start ( vArgumentList, pFormatStr );

//        m_pFunctions->pLogPrintf( this, bTestCaseState, pFormatStr, vArgumentList );

//        va_end ( vArgumentList );
//    }
//} // </method_logPrintf>
 */

