         EXTRN   __AHINCR:abs
         .MODEL  LARGE
STARTWS0_SEG SEGMENT WORD PUBLIC 'STARTWS_CODE'

         PUBLIC  _SegIncr
_SegIncr PROC
         MOV AX, __AHINCR
         RET
_SegIncr ENDP

STARTWS0_SEG ENDS
         END
