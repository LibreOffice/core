; 
; DESCRIPTION
;      This code is a model independent version of DOS exec that will swap
;      the calling process out to secondary storage prior to running the
;      child.  The prototype for calling the exec function is below.
;
;      exec( int swap, char far *program, char far *cmdtail,
;	     int environment_seg, char far *tmpfilename );
;
;
;      To assemble this file issue the command:
;
;         tasm /mx /t /dmmodel exec.asm
;
;      where 'model' is one of {small, compact, medium, large}, you may
;      also use MASM 5.1 to assemble this file, in this case simply replace
;      'tasm' with 'masm' in the above command line.
;
; AUTHOR
;      Dennis Vadura, dvadura@watdragon.uwaterloo.ca
;      CS DEPT, University of Waterloo, Waterloo, Ont., Canada
;
; COPYRIGHT
;      Copyright (c) 1990 by Dennis Vadura.  All rights reserved.
; 
;      This program is free software; you can redistribute it and/or
;      modify it under the terms of the GNU General Public License
;      (version 1), as published by the Free Software Foundation, and
;      found in the file 'LICENSE' included with this distribution.
; 
;      This program is distributed in the hope that it will be useful,
;      but WITHOUT ANY WARRANTY; without even the implied warrant of
;      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;      GNU General Public License for more details.
; 
;      You should have received a copy of the GNU General Public License
;      along with this program;  if not, write to the Free Software
;      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;
ifdef have286
 .286    ; define have286 with -D for 80286 processor or better
    mpusha Macro
    	pusha
    Endm

    mpopa Macro
    	popa
    Endm

else	; 8088/8086 compatible
    mpusha Macro
    	push ax
	push cx
	push dx
	push bx
	push sp
	push bp
	push si
	push di
    Endm

    mpopa Macro
    	pop di
	pop si
	pop bp
	add sp,2
	pop bx
	pop dx
	pop cx
	pop ax
    Endm
endif

ifdef msmall
	        .model	small
argbase		equ	4
endif
ifdef mcompact
		.model  compact
argbase		equ	4
endif
ifdef mmedium
		.model	medium
argbase		equ	6
endif
ifdef mlarge
		.model	large
argbase		equ	6
endif
a_swap		equ	<bp+argbase+0>
a_prog		equ	<bp+argbase+2>
a_tail		equ	<bp+argbase+6>
a_env 		equ	<bp+argbase+10>
a_tmp		equ	<bp+argbase+12>

a_handle	equ	<bp+argbase>


; Define all useful equ's
swap_xms	equ	0		; we swapped it out to xms
swap_ems	equ	2		; we swapped it out to ems
swap_file	equ	4		; we swapped it out to a file
seg_no_alloc	equ	0		; this is part of a segment
seg_alloc	equ	1		; this is a full segment header
seg_data	equ	2		; this is data for part of a segment


; Define any global/external variables that we will be accessing from here.
	        .data
		extrn	_errno:word		; Set to dos ret code from exec
		public  _Interrupted		; Set to 1 if interrupted 0
_Interrupted	dw	0			; otherwise

	        .code
		assume	cs:@code, ds:@code, ss:@code, es:@code

		even
execstack	dw	64  dup (?)	; put the temporary exec stack right
exec_sp		label	word		; at the start.

old_ss		dw	?		; save stack seg across exec
old_sp		dw	?		; save stack ptr across exec
progsize	dw	?		; original size of the program
rootsize	dw	?		; size of base root kept during swap
resend		dw	?		; paragraph where resident code ends
envseg		dw	?		; paragraph of environment segment
psp		dw	?		; our own psp
swap		dw	?		; swapping selection flag
eretcode	dw	?		; return code from exec
interrupted	dw	?		; interrupted flag for exec
arenahead	dw	?		; start of memory block list
alstr		dw	?		; allocation strategy save spot
in_exec		dw	0		; flag, 1 ==> in exec

cmdpath		db	65  dup(?)	; file to exec
cmdtail		db	129 dup(?)	; its command tail
fcb		db	37  dup(0)	; dummy fcb
tmpseg		db	7   dup(?)	; block header buffer

tmpname		db	65  dup(0)	; name of temporary file resource

		even
tmphandle	dw	?		; handle for temporary file
real_21h	dd	0		; will be DOS's 21h vector if doing -C

std_fil_handle	dw	?		; file handle for -C file
std_fil_number	db	?		; system file number for -C file
our_stdout	db	?		; sys file number our stdout handle

error_rhdr	db	"exec: Failure reading header block", 0DH, 0AH, '$'
error_rseg	db	"exec: Failure reading segment data", 0DH, 0AH, '$'
error_resize	db	"exec: Failure on resize", 0DH, 0AH, '$'
error_free	db	"exec: Failure to free a block", 0DH, 0AH, '$'
error_string	db	"exec: Program swap failure", 0DH, 0AH, '$'
error_alloc	db	"exec: Memory blocks don't match", 0DH, 0AH, '$'

		even
write_header label word
   whdr_xms_ptr		dw	word ptr whdr_xms
   whdr_ems_ptr		dw	word ptr whdr_ems
   whdr_file_ptr	dw	word ptr whdr_file

write_seg label word
   wseg_xms_ptr		dw	word ptr wseg_xms
   wseg_ems_ptr		dw	word ptr wseg_ems
   wseg_file_ptr	dw	word ptr wseg_file

read_header label word
   rhdr_xms_ptr		dw	word ptr rhdr_xms
   rhdr_ems_ptr		dw	word ptr rhdr_ems
   rhdr_file_ptr	dw	word ptr rhdr_file

read_seg label word
   rseg_xms_ptr		dw	word ptr rseg_xms
   rseg_ems_ptr		dw	word ptr rseg_ems
   rseg_file_ptr	dw	word ptr rseg_file

free_resource label word
   free_xms_ptr		dw	word ptr free_xms_resource
   free_ems_ptr		dw	word ptr free_ems_resource
   free_file_ptr	dw	word ptr free_file_resource

reset_resource label word
   reset_xms_ptr	dw	word ptr reset_xms_resource
   reset_ems_ptr	dw	word ptr reset_ems_resource
   reset_file_ptr	dw	word ptr reset_file_resource

old_ctl_brk label dword
   old_ctl_brk_off	dw	?
   old_ctl_brk_seg 	dw	?

old_crit_err label dword
   old_crit_err_off	dw	?
   old_crit_err_seg 	dw	?

exec_block label word
  ex_envseg	dw	?			; env seg, use parent's if 0
  ex_cmdtail	dd	?			; command tail for exec
  ex_fcb1	dd	far ptr fcb		; fcb's aren't used by dmake
  ex_fcb2	dd	far ptr fcb
  ex_ss		dw	?			; saved ss for exec
  ex_sp		dw	?			; saved sp for exec
  ex_error	dw	0			; error code for dos exec


; Special 21h (DOS call) handler to tee stdout/stderr writes to the -C file.
; Ignore 21h calls that aren't writes to 1 or 2; i.e., pass them to DOS handler.
; If write call was from this process, it's pretty simple to duplicate it
; to the -C file.  If it's from another process, we try to write to its
; inherited handle.  Worst case is where the handle wasn't inherited: someone
; closed it.  In that instance we have to switch to dmake's PSP to do the
; duplicate write.

; Subprocesses do not get their stdout/stderr teed to the -C file if
; their stdout/stderr no longer points to the file/device that dmake's
; stdout points to.  This is tested by looking at the process's job
; file table, which is a table that maps process handles to DOS system file
; table numbers.  (The far pointer to the JFT is at the PSP offset 34h.)
; The JFT is also queried to see if the -C file was inherited.

; O_BINARY, O_TEXT problems are ignored here.  These are fudged by the
; C library before it calls DOS; since we're working below that level
; we don't have to worry about it.

simulate_21h Macro
    pushf			;; direct call to DOS
    call cs:[real_21h]
    Endm

	assume cs:@code, ds:nothing, es:nothing, ss:nothing
our_21h_handler proc far
    pushf
    cmp ah,40h		; is this a write?
    jne call_dos	; --no
    cmp bx,1		; write on handle 1 (stdout?)
    je duplicate_it
    cmp bx,2		; stderr?
    je duplicate_it

call_dos:
    popf
    jmp [real_21h]	; far jump to real handler, which will do the sys call
    			; and return to the original caller

duplicate_it:
    mpusha
    push ds
    push es
    mov bp,sp

    mov di,std_fil_handle	; handle of the -C file

  If @CodeSize eq 0
  	; Small/compact models allow for quick test of us versus subprocess.
	; False negative (it's us with a different CS) will be picked
	; up by code just below.  (Might happen due to call from C library.)
	; False positives would be bad, but can't happen.
    mov ax,[bp+24]	; caller's CS
    cmp ax,@code	; same as us?
    je call_from_dmake
  Endif

    mov ah,51h		; get PSP ("undocumented version" works in DOS 2.0+)
    simulate_21h	; PSP segment returned in BX
    cmp bx,psp		; our PSP?
    je call_from_dmake	; --yes, no PSP changing needed

    mov es,bx		; set ES to current (caller's) PSP
    lds bx,es:[34h]	; set DS:BX pointing to caller's job file table

    mov si,[bp+12]	; file handle caller passed in (known to be 1 or 2)
    mov al,[bx+si]	; system file number corresponding to caller's handle
    cmp al,our_stdout	; same as our stdout?
    jne do_real_write	; no--subprocess must have redirected it

    mov al,[bx+di]	; see if caller has dup of -C file still open
    cmp al,std_fil_number
    je use_dup		; yes--we can write using caller's PSP

    	; Calling process (or some intermediate process) has closed
	; the -C descriptor.  We'll use dmake's (our) -C descriptor, but
	; to do so we'll have to change the PSP.  Disable BREAK handling
	; so that ^break doesn't kill the wrong process.

    mov ax,3300h	; get BREAK flag
    simulate_21h
    mov si,dx		; save BREAK state in SI
    sub dx,dx		; now turn break flag off
    mov ax,3301h
    simulate_21h	; don't want ^Break recoginized while PSP changed
    mov bx,psp		; set dmake's PSP
    mov ah,50h
    simulate_21h

    mov bx,di			; handle of -C file
    ; CX still has caller's count
    mov ds,[bp+2]		; restore caller's DS
    mov dx,[bp+14]		; DS:DX again points to caller's buffer
    mov ah,40h
    simulate_21h		; write the copy

    mov bx,es		; caller's PSP
    mov ah,50h		; set PSP
    simulate_21h	; restore caller's PSP
    mov dx,si		; break state before we changed it
    mov ax,3301h
    simulate_21h	; restore break state

    jmp short do_real_write

use_dup:
    mov ds,[bp+2]		; restore caller's DS
    mov dx,[bp+14]		; DS:DX again points to caller's buffer

call_from_dmake:
    mov bx,di			; handle of -C file
    mov ah,40h			; write
    ; CX still has caller's count
    simulate_21h		; write to the file

do_real_write:
    pop es
    pop ds
    mpopa
    popf
    jmp [real_21h]	; far jump to real handler, which will do the sys call
    			; and return to the original caller
our_21h_handler endp

	assume	cs:@code, ds:@code, ss:@code, es:@code

;-----------------------------------------------------------------------------
; First define the critical-error and control-brk handlers. 
; The critical error handler simply pops the machine state and returns an
; access denied result code.
crit_err_handler proc far
		add	sp, 6		; ip/cs/flags ...
		pop	ax
		pop	bx
		pop	cx
		pop	dx
		pop	si
		pop	di
		pop	bp
		pop	ds
		pop	es
		push	bp		; fix up the return flags
		mov	bp, sp
		xchg	ax, [bp+6]	; get the flag byte.
		or	ax, 1		; set the carry bit
		xchg	ax, [bp+6]	; put it back.
		pop	bp
		mov	ax, 5		; access denied
		iret
crit_err_handler endp


;-----------------------------------------------------------------------------
; Here we set the interrupted flag, and terminate the currently running
; process.
ctl_brk_handler proc far
		clc				; make sure carry is clear
		inc	cs:interrupted		; set the flag

; Make certain it isn't us that is going to get terminated.
; There is a small window where the in_exec flag is set but the child is
; not running yet, I assume that DOS doesn't test for ctl_brk at that time
; as it is bussily creating a new process.
		cmp	cs:in_exec,0
		je	just_return		; note this implies CF == 0
		stc				; set CF to abort child
just_return:	iret
ctl_brk_handler endp


;-----------------------------------------------------------------------------
; Something really nasty happened, so abort the exec call and exit.
; This kills the calling process altogether, and is a very nasty way of
; termination since files may still be open etc.
abort_exec_rhdr label near
		mov	dx, offset error_rhdr
		jmp	print_it
abort_exec_rseg label near
		mov	dx, offset error_rseg
		jmp	print_it
abort_exec_resize label near
		mov	dx, offset error_resize
		jmp	print_it
abort_exec_free label near
		mov	dx, offset error_free
		jmp	print_it
abort_exec_alloc label near
		mov	dx, offset error_alloc
		jmp	print_it
abort_exec proc near
		mov	dx, offset error_string
print_it:	push	dx
		mov	bx, [swap]
		call	[free_resource+bx]
		mov	ax, cs
		mov	ds, ax
		pop	dx
		mov	ah, 9
		int	21H
kill_program:	mov	ax, 04cffH			; nuke it!
		int	21H
abort_exec endp


;-----------------------------------------------------------------------------
; lodsw/stosw loop to copy data.  Called only for word copy operations.
; 	ds:si  - point at source
;	es:di  - point at destination
;	cx     - count of bytes to copy.
copy_data proc near
		shr	cx, 1		; convert to word count
		jnc	copy_words
		movsb
copy_words:	rep	movsw		; copy the words.
		ret
copy_data endp



;=============================================================================
; THE FOLLOWING SECTION DEALS WITH ALL ROUTINES REQUIRED TO READ XMS RECORDS.
;=============================================================================
rhdr_xms proc near
		ret
rhdr_xms endp

rseg_xms proc near
		ret
rseg_xms endp

reset_xms_resource proc near
		ret
reset_xms_resource endp

free_xms_resource proc near
		ret
free_xms_resource endp
;=============================================================================



;=============================================================================
; THE FOLLOWING SECTION DEALS WITH ALL ROUTINES REQUIRED TO READ EMS RECORDS.
;=============================================================================
rhdr_ems proc near
		ret
rhdr_ems endp

rseg_ems proc near
		ret
rseg_ems endp

reset_ems_resource proc near
		ret
reset_ems_resource endp

free_ems_resource proc near
		ret
free_ems_resource endp
;=============================================================================



;=============================================================================
; THE FOLLOWING SECTION DEALS WITH ALL ROUTINES REQUIRED TO READ FILE RECORDS.
;=============================================================================
; This routine reads a segment header from a file.
; The header is a seven byte record formatted as follows:
;	segment address		- of data
;	offset address		- of data
; 	length in paragraphs	- of data
;	mode			- 1 => segment header (allocate seg on read)
;				  0 => subsegment, don't allocate on read.
; The information is placed into the tmpseg data area in the code segment.
; The routine aborts if an error is detected.
rhdr_file proc near
		mov	dx, offset tmpseg	; read the header record out
		mov	cx, 7
		mov	bx, [tmphandle]
		mov	ah, 03fH
		int	21H
		jnc	rhdr_done		; make sure it worked
		jmp	abort_exec_rhdr

rhdr_done:	cmp	ax, 7
		je	exit_rhdr_file
		or	ax, ax
		je	signal_eof
		jmp	abort_exec_rhdr

signal_eof:	stc
exit_rhdr_file:	ret
rhdr_file endp


;-----------------------------------------------------------------------------
; Read a segment from the temporary file whose handle is in cs:tmphandle.
; The routine aborts if an error is detected.
rseg_file proc near
		push	ds
		mov	ds, word ptr cs:tmpseg; Now read the whole segment
		mov	dx, word ptr cs:tmpseg+2
		mov	cx, word ptr cs:tmpseg+4
		mov	bx, cs:tmphandle
		mov	ah, 03fH
		int	21H
		pop	ds
		jnc	rseg_done
		jmp	abort_exec_rseg

rseg_done:	cmp	ax, [word ptr tmpseg+4]
		je	exit_rseg_file
		jmp	abort_exec_rseg		; If we didn't get read full
exit_rseg_file:	ret				; segment then abort
rseg_file endp


;-----------------------------------------------------------------------------
; Seek to the beginning of the file.
reset_file_resource proc near
		mov	bx, [tmphandle]
		xor	cx, cx
		mov	dx, cx
		mov	ax, 04200H		; seek to begining of file
		int	21H
		ret
reset_file_resource endp


;-----------------------------------------------------------------------------
; unlink the temporary file allocated for swapping.
; We close the file first, and then delete it.   We ignore errors here since
; we can't do anything about them anyway.
free_file_resource proc near
		mov	bx, [tmphandle]		; get the file handle
		mov	ah, 03eH		; close the file
		int	21H
		mov	dx, offset tmpname	; Now delete the temp file
		mov	ah, 041H
		int	21H
		ret
free_file_resource endp
;=============================================================================



;=============================================================================
; CODE TO SWAP THE IMAGE IN FROM SECONDARY STORAGE
;=============================================================================
swap_in proc near
		mov	bx, [alstr]		; get previous alloc strategy
		mov	ax, 5801H		; and set it back
		int	21H
		mov	bx, [swap]		; get type of resource
		call	[reset_resource+bx]	; reset the resource
		mov	es, [psp]		; resize the program back
		mov	bx, [progsize]		; to original size
		mov	ah, 04AH
		int	21H
		jnc	read_seg_loop
		jmp	abort_exec

read_seg_loop:	mov	bx, [swap]		; get type of resource
		call	[read_header+bx]	; get seg header
		jc	exit_swap_in		; all done
		mov	al, [tmpseg+6]
		cmp	al, seg_no_alloc	; see if dummy segment header
		je	read_seg_loop
		cmp	al, seg_alloc		; do we need to do an alloc?
		jne	read_data		; nope

; Allocate back the memory for a segment that is not the [psp], note that this
; must come back to the same segment we had previously since other segments
; may have pointers stored in their variables that point to this segment using
; segment:offset long pointers.
		mov	bx, [word ptr tmpseg+4]	; get count of paragraphs
		mov	ah, 048H		; dos_alloc
		int	21H
		jc	alloc_error		; oops!
		cmp	ax, [word ptr tmpseg]	; did we get the same segment?
		je	read_seg_loop		; yup!
alloc_error:	jmp	abort_exec_alloc

read_data:	mov	bx, [swap]
		call	[read_seg+bx]		; this must succeed, if fail
		jmp	read_seg_loop		; we never come back here

exit_swap_in:	mov	bx, [swap]		; all done, so free resource
		call	[free_resource+bx]
		ret
swap_in endp


;=============================================================================
; CODE TO SWAP THE IMAGE OUT TO SECONDARY STORAGE
;=============================================================================
; This routine is called to swap the non-resident portion of the program
; out to the resource specified by the value of [cs:swap].  If the swap out
; fails, then appropriate routines are called to free the resources allocated
; up to that point.
;
; The steps used to swap the program out are as follows:
;	- calculate new size of program to remain resident and size to swap
;	  out.
;	- write out non-resident portion of current segment
;	- walk DOS allocation chain and write out all other segments owned by
;	  the current program that are contiguous with the _psp segment
;	- copy the environment down to low memory
;	- resize the current _psp segment to savesize
;	- free all segments belonging to program except current _psp segment
swap_out proc near
		mov	ax, 05800H	; get memory alocation strategy
		int	021H
		mov	[alstr], ax	; and save it for future restoration.
		mov	di, [psp]	; compute length of program to current
		mov	bx, cs		; value of cs, and find program size
		sub	bx, di		; by looking at length stored in
		mov	ax, di		; arena header found in front of psp
		dec	ax
		mov	es, ax
		mov	si, es:3	; si is size of program in paragraphs
		mov	[progsize], si	; progsize now contains the size.

; Now compute length of program segment to save.
; Length is:   cs - psp + (offset overlay_code_here+15 >> 4)
		mov	ax, offset overlay_code_here+15
		shr	ax, 1
		shr	ax, 1
		shr	ax, 1
		shr	ax, 1
		add	bx, ax			; bx is size of program to keep
		sub	si, bx			; si is # of paragraphs to save.
		add	di, bx			; di is paragraph to start at
		mov	rootsize, bx
		mov	resend, di		; cs:resend is saved start para
		mov	al, seg_no_alloc	; set no allocation for segment
		call	write_segment
		jc	abort_swap_out

; We have now saved the portion of the program segment that will not remain
; resident during the exec.  We should now walk the DOS allocation chain and
; write out all other segments owned by the current process.
save_segments:	mov	ax, [psp]
		dec	ax
		mov	es, ax
		mov	bx, offset write_segment_data
		call	walk_arena_chain
		jc	abort_swap_out

; Now we must walk the chain of allocated memory blocks again and free
; all those that are owned by the current process, except the one that is
; the current process' psp.
free_segments:	mov	ax, [psp]
		dec	ax
		mov	es,ax
		mov	bx, offset free_dos_segment
		call	walk_arena_chain
		jnc	resize_program
		jmp	abort_exec_free		; can't fix it up now.

; We now resize the program to the size specified by cs:rootsize.  This will
; free most of the memory taken up by the current program segment.
resize_program: mov	es, [psp]		; es is segment to resize.
		mov	bx, [rootsize]		; bx is size of segment.
		mov	ah, 04aH		; resize memory block
		int	21H
		jnc	swap_out_ok
		jmp	abort_exec_resize	; disaster
swap_out_ok:	ret

; The swap out failed for some reason, so free any allocated resources
; and set the carry bit.
abort_swap_out:	mov	bx, [swap]
		call	[free_resource+bx]
		xor	ax, ax
		mov	[swap], ax		; clear the swap flag
		stc
		ret
swap_out endp


;=============================================================================
; CODE TO SET-UP FOR AND EXEC THE CHILD PROCESS
;=============================================================================
; Actually execute the program.  If cs:swap is set, this code will invoke the
; swap-out/swap-in code as required.
do_exec proc near
		cmp	[swap], 0		; does the user want to swap?
		je	no_swap_out		; nope
		call	init_swap		; figger out where to swap to
		jc	no_swap_out		; if carry set then don't swap
		call	swap_out

no_swap_out:	cmp	[interrupted], 0	; were we interrupted?
		jne	leave_exec		; yep, so clean up, don't exec

; free passed in environment block if it is non zero.
; This way the parent program does not need to free it.
		mov	ax, [envseg]
		or	ax, ax
		je	setup_block
		push	ax
		mov	es, ax
		mov	ah, 49H
		int	21H
		pop	ax

; set up the parameter block for the DOS exec call.
;    offset  contents
;        00  segment address of environment to be passed,
; 	     0 => use parents env.
;        02  pointer to command tail for new process.
;        06  pointer to fcb1
;        0a  pointer to fcb2
setup_block:	mov	ax, [envseg]
		mov	[ex_envseg], ax
		mov	cx, cs
		mov	[word ptr ex_cmdtail], offset cmdtail
		mov	[word ptr ex_cmdtail+2], cx

; set up registers for exec call
;	ds:dx	- pointer to pathname of program to execute
;	es:bx	- pointer to above parameter block
		mov	dx, offset cmdpath
		mov	es, cx
		mov	bx, offset exec_block

; Under DOS 2.x exec is notorious for clobbering registers and guarantees
; to preserve only cs:ip.
		push	ds
		mov	[ex_sp], sp
		mov	[ex_ss], ss
		mov	[ex_error], 0		; clear exec error code
		inc	[in_exec]		; set internal flag
		mov	ax, 04b00H
		int	21H

; returned from exec, so restore possibly clobbered registers.
		mov	ss, cs:ex_ss
		mov	sp, cs:ex_sp
		pop	ds

; check to make certain the exec call worked.
		jnc	it_worked

; exec call failed.  Save return code from msdos.
		mov	[ex_error], ax
		jmp	leave_exec

it_worked:	mov	ah, 04dH	; get the return code
		int	21H
		cmp	ah,1		; check if terminated by ^C
		jnz	nosigint
		inc	interrupted	; yes so set flag
nosigint:	xor	ah, ah		; 8-bit return code, so clear ah
		mov	[eretcode], ax

leave_exec:	cmp	[swap], 0	; check swap, if non-zero swap back in
		je	no_swap_in
		call	swap_in

; Clear the in_exec after the swap back in.  This way we are guaranteed to
; get parent in and the resources freed should a ^C be hit when we are reading
; the image in.
no_swap_in:	mov	[in_exec], 0
		ret
do_exec endp				



;==============================================================================
; Everything past this point is overwriten with the environment and new
; program after the currently executing program is swapped out.
;==============================================================================
overlay_code_here label word

;-----------------------------------------------------------------------------
; Figure out where we can swap to and initialize the resource we are going to
; use.  We try XMS, EMS, and a tempfile (if specified), in that order.  We set
; [cs:swap] to the correct value based on which of the resources exists.
; If none can be used, then [cs:swap] is set to 0, and no swap takes place.
; The exec code will still attempt to execute the child in this instance, but
; may fail due to lack of resources.   Each swap_out_* routine must provide
; its own clean-up handler should it not be able to write all program
; segments to the swap resource.
init_swap proc near
		mov	[swap], 0
;call	init_xms
;jnc	init_done
;call	init_ems
;jnc	init_done
		call	init_file
init_done:	ret
init_swap endp


;-----------------------------------------------------------------------------
; This routine is used to walk the DOS allocated memory block chain
; starting at address supplied in the es register.  For each block it
; calls the routine specified by the bx register with the segment length
; in si, and its address in di.  It does not apply the routine to the
; segment if the segment is the same as the current program's [cs:psp] value.
memheader struc
   magic	db	?	; either 'Z' for end or 'M' for allocated
   owner	dw	?	; psp of owner block
   len		dw	?	; length in paragraphs of segment
memheader ends

walk_arena_chain proc near
		mov	si, word ptr es:3		; get length
		mov	di, es
		inc	di
		mov	ax, word ptr es:1

; Stop the search if the block is NOT owned by us.  Ignore our own psp block
; and our environment segment block.
		cmp	ax, cs:psp			; is it owned by us?
		jne	walk_done			; NOPE!  -- all done
		cmp	di, cs:envseg			; skip our environment
		je	next_block
		cmp	di, cs:psp			; skip our psp
		je	next_block

; Now save state and call the routine pointed at by [bx].
		push	di
		push	si
		push	bx
		call	bx
		pop	bx
		pop	si
		pop	di
		jc	exit_walk			; if error then stop
		mov	al, byte ptr es:0		; check if at end 
		cmp	al, 'Z'
		je	walk_done

next_block:	add	di, si				; go on to next segment
		mov	es, di
		jmp	walk_arena_chain
walk_done:	clc
exit_walk:	ret
walk_arena_chain endp


;-----------------------------------------------------------------------------
; This routine takes a dos segment found in the di register and free's it.
free_dos_segment proc near
		mov	es, di		; free dos memory block
		mov	ah, 49H
		int	21H
		ret
free_dos_segment endp


;-----------------------------------------------------------------------------
; Called to invoke write_segment with proper values in the al register.  Only
; ever called from walk_arena_chain, and so al should be set to seg_alloc.
write_segment_data label near
		mov	al, seg_alloc	; and fall through into write_segment
;-----------------------------------------------------------------------------
; This routine writes a segment as a block of data segments if the number of
; paragraphs to write exceeds 0x0fff (rarely the case).
; It stuffs the info into tmpseg, and then calls wheader and wseg to get the
; data out.
;
;	di:dx	segment:offset of segment;  offset is ALWAYS zero.
;	si	number of paragraphs to write.
;	al	mode of header to write
write_segment proc near
		push	di
		push	si
		xor	dx,dx
		mov	bx, [swap]
		call	[write_header+bx]
		pop	si
		pop	di
		jc	exit_wseg

do_io_loop:	cmp	si, 0		; are we done yet?
		je	exit_wseg	; yup so leave.
		mov	cx, si		; # of paragraphs to move
		cmp	cx, 0fffH	; see if we have lots to move?
		jle	do_io
		mov	cx, 0fffH	; reset to max I/O size

do_io:		push	cx		; save # of paragraphs we are writing
		shl	cx, 1		; shift cx by four to the left
		shl	cx, 1
		shl	cx, 1
		shl	cx, 1
		push    di		; save the start, and count left
		push    si
		mov	si, cx
		xor	dx,dx
		mov	al, seg_data
		mov	bx, [swap]
		push	bx
		call	[write_header+bx]
		pop	bx
		call	[write_seg+bx]
		pop	si
		pop	di
		pop	dx		; original paragraph count in dx
		jc	exit_wseg	; it failed so exit.
		add	di, dx		; adjust the pointers, and continue.
		sub	si, dx
		jmp     do_io_loop
exit_wseg:	ret
write_segment endp


;=============================================================================
; THE FOLLOWING SECTION DEALS WITH ALL ROUTINES REQUIRED TO WRITE XMS RECORDS.
;=============================================================================
init_xms proc near
		ret
init_xms endp

whdr_xms proc near
		ret
whdr_xms endp

wseg_xms proc near
		ret
wseg_xms endp
;=============================================================================


;=============================================================================
; THE FOLLOWING SECTION DEALS WITH ALL ROUTINES REQUIRED TO WRITE EMS RECORDS.
;=============================================================================
init_ems proc near
		ret
init_ems endp

whdr_ems proc near
		ret
whdr_ems endp

wseg_ems proc near
		ret
wseg_ems endp
;=============================================================================


;=============================================================================
; THE FOLLOWING SECTION DEALS WITH ALL ROUTINES REQUIRED TO WRITE FILES.
;=============================================================================
;-----------------------------------------------------------------------------
; Attempt to create a temporary file.  If the tempfile name is NIL then return
; with the cary flag set.
init_file proc near
		mov	al, [tmpname]
		or	al, al
		je	err_init_file
		mov	dx, offset tmpname
		xor 	cx, cx
		mov	ah, 03cH
		int	21H
		jc	err_init_file		; if carry set then failure
		mov	[tmphandle], ax		; init swapping
		mov	[swap], swap_file
		jmp	exit_init_file
err_init_file:	stc
exit_init_file: ret
init_file endp


;-----------------------------------------------------------------------------
; This routine writes a segment header to a file.
; The header is a seven byte record formatted as follows:
;	segment address		- of data
;	offset address		- of data
; 	length in paragraphs	- of data
;	mode			- 1 => segment header (allocate seg on read)
;				  0 => subsegment, don't allocate on read.
; Routine takes three arguments:
;	di:dx	segment:offset of segment
;	si	number of paragraphs to write.
;	al	mode of header to write
whdr_file proc near
		mov	[word ptr tmpseg], di	; save the segment/offset
		mov	[word ptr tmpseg+2], dx
		mov	[word ptr tmpseg+4], si	; save the segment length
		mov	[tmpseg+6], al
		mov	dx, offset tmpseg	; write the header record out
		mov	cx, 7
		mov	bx, [tmphandle]
		mov	ah, 040H
		int	21H
		jc	exit_whdr_file		; make sure it worked
		cmp	ax, 7
		je	exit_whdr_file		; oh oh, disk is full!
err_whdr_file:	stc
exit_whdr_file:	ret
whdr_file endp


;-----------------------------------------------------------------------------
; Write a segment to the temporary file whose handle is in cs:tmphandle
; Parameters for the write are assumed to be stored in the tmpseg data area.
; function returns carry set if failed, carry clear otherwise.
wseg_file proc near
		push	ds
		mov	ds, word ptr cs:tmpseg ; Now write the whole segment
		mov	dx, word ptr cs:tmpseg+2
		mov	cx, word ptr cs:tmpseg+4
		mov	bx, cs:tmphandle
		mov	ah, 040H
		int	21H
		pop	ds
		jc	exit_wseg_file		; make sure it worked
		cmp	ax, [word ptr tmpseg+4]
		je	exit_wseg_file
err_wseg_file:	stc				; it failed (usually disk full)
exit_wseg_file:	ret
wseg_file endp
;=============================================================================


;=============================================================================
; _exec: THIS IS THE MAIN ENTRY ROUTINE TO THIS MODULE
;=============================================================================
; This is the main entry routine into the swap code and corresponds to the
; following C function call:
;
; exec( int swap, char far *program, char far *cmdtail, int environment_seg,
;	char far *tmpfilename );
;
; Exec performs the following:
;	1. set up the local code segment copies of arguments to the exec call.
;	2. switch to a local stack frame so that we don't clobber the user
;	   stack.
;	3. save old interrupt vectors for ctrl-brk.
;	4. install our own handler for the ctrl-brk interrupt, our handler
;	   terminates the current running process, and returns with non-zero
;	   status code.
;	5. get our psp
;	6. setup arguments for exec call
;	7. exec the program, save result code on return.
;       8. restore previous ctrl-brk and crit-error handler.
;       9. restore previous process stack, and segment registers.
;      10. return from exec with child result code in AX
;	   and global _Interrupted flag set to true if child execution was
;	   interrupted.

; NOTE:  When first called the segments here assume the standard segment
;        settings.
		assume cs:@code, ds:DGROUP,es:DGROUP,ss:DGROUP

		public	_exec
_exec proc
	    	push	bp		; set up the stack frame
		mov	bp, sp
		push	si		; save registers we shouldn't step on.
		push	di
		push	ds

; set up for copying of parameters passed in with long pointers.
		push	cs		; going to use lodsb/stosb, set up es
		pop	es		; as destination.
		assume  es:@code	; let the assembler know :-)
		cld			; make sure direction is right

; Copy all parameters into the bottom of the code segment.  After doing so we
; will immediately switch stacks, so that the user stack is preserved intact.
		mov	ax, ss:[a_swap]		; save swap
		mov	es:swap, ax
		mov	ax, ss:[a_env]		; save env seg to use
		mov	es:envseg, ax

		mov 	di, offset cs:cmdpath	; copy the command
		lds 	si, ss:[a_prog]		; 65 bytes worth
		mov	cx, 65
		call	copy_data

		mov	di, offset cs:cmdtail	; copy the command tail
		lds	si, ss:[a_tail]		; 129 bytes worth
		mov	cx, 129
		call	copy_data

		mov	di, offset cs:tmpname	; copy the temp file name
		lds	si, ss:[a_tmp]		; 65 bytes worth.
		mov	cx, 65
		call	copy_data

; Now we save the current ss:sp stack pointer and swap stack to our temporary
; stack located in the current code segment.  At the same time we reset the
; segment pointers to point into the code segment only.
swap_stacks:	mov	ax, ss
		mov	es:old_ss, ax
		mov	es:old_sp, sp
		mov	ax, cs
		mov	ds, ax
		mov	ss, ax			; set ss first, ints are then
		mov	sp, offset cs:exec_sp	; disabled for this instr too
		assume  ds:@code, ss:@code	; let the assembler know :-)

; Now we save the old control break and critical error handler addresses.
; We replace them by our own routines found in the resident portion of the
; swapping exec code.
set_handlers:	mov	[interrupted], 0	; clear interrupted flag
		mov	[eretcode], 0		; clear the return code
		mov	ax, 03523H		; get int 23 handler address
		int	21H
		mov	cs:old_ctl_brk_off, bx
		mov	cs:old_ctl_brk_seg, es
		mov	dx, offset ctl_brk_handler
		mov	ax, 02523H		; set int 23 handler address
		int	21H

		mov	ax, 03524H		; get int 24 handler address
		int	21H
		mov	cs:old_crit_err_off, bx
		mov	cs:old_crit_err_seg, es
		mov	dx, offset crit_err_handler
		mov	ax, 02524H		; set int 24 handler address
		int	21H

; Go and execute the child, we've set up all of its parameters.  The do_exec
; routine will attempt to perform a swap of the code if requested to do so by
; a non-zero value in the variable cs:swap.
		mov	ah, 051H		; get the psp
		int	21H
		mov	cs:psp, bx
		call	do_exec

; We're back from the exec, so fix things up the way they were.
; Restore the old control-break and critical-error handlers.
		lds	dx, cs:old_ctl_brk
		mov	ax, 02523H
		int	21H
		lds	dx, cs:old_crit_err
		mov	ax, 02524H
		int	21H

; Restore previous program stack segment registers, and data segment.
		mov	ax, cs:old_ss
		mov	ss, ax			; mov into ss first, that way
		mov	sp, cs:old_sp		; no interrupts in this instr.
		pop	ds

; Tell the assembler we have swaped segments again.
		assume	ds:DGROUP,es:DGROUP,ss:DGROUP

; Set the global Interrupted flag so that parent can tell it was interrupted.
		mov	ax, seg DGROUP:_Interrupted
		mov	es, ax
		mov	ax, cs:interrupted
		mov	es:_Interrupted, ax

; Set the global errno value to reflect the success/failure of the DOS
; exec call.
		mov	ax, seg DGROUP:_errno
		mov	es, ax
		mov	ax, cs:ex_error
		mov	es:_errno, ax

; Fetch the child's return code, pop rest of stuff off of the stack
; and return to the caller.
		mov	ax, cs:eretcode
		pop	di
		pop	si
		pop	bp
		ret
_exec endp

; void do_hook_std_writes(int handle);
;	This saves the 21h interrupt vector and changes it to point
;	into this code.  Argument is the file handle of the -C file.

    public _do_hook_std_writes
_do_hook_std_writes proc
    		push	bp
		mov	bp,sp
		push	di

		mov	di, ss:[a_handle]	; handle of -C file
		mov	std_fil_handle, di

		mov	ah, 51h			; request our PSP
		int	21h
		mov	[psp], bx		; save it

		mov	es, bx
		les	bx, es:[34h]		; pointer to job file table
		mov	al, es:[bx+1]		; system file # of our stdout
		mov	[our_stdout], al
		mov	al, es:[bx+di]		; system file number of -C file
		mov	std_fil_number, al

		mov	ax,3521h		; request vector 21h
		int	21h			; it's returned in ES:BX
		mov	word ptr [real_21h], bx
		mov	word ptr [real_21h+2], es

		push	ds
		mov	ax,cs
		mov	ds,ax
		lea	dx,our_21h_handler	; DS:DX is the new vector
		mov	ax,2521h		; set vector 21h
		int	21h

		pop	ds
		pop	di
		pop	bp
		ret
_do_hook_std_writes endp

; void do_unhook_std_writes(void);
;	This restores the 21h interrupt vector.
;	The saved vector is zero if it wasn't changed (no -C option).

    public _do_unhook_std_writes
_do_unhook_std_writes proc
		push	ds

    		lds	dx, [real_21h]	; put saved vector into DS:DX
		mov	ax, ds
		or	ax, dx
		jz	unhook_return	; zero means we didn't hook 21h

		mov	ax,2521h	; set vector 21h
		simulate_21h

unhook_return:	pop ds
		ret
_do_unhook_std_writes endp
end
